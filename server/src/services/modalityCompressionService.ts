import { execFile } from 'child_process';
import { promisify } from 'util';
import { existsSync } from 'fs';
import { rm, stat } from 'fs/promises';
import path from 'path';
import { config } from '../config/index.js';
import { AlgorithmType, ModalityType, CompressionMetrics } from '../types/index.js';

const execFileAsync = promisify(execFile);

export class ModalityCompressionService {
  static async run(
    operation: 'compress' | 'decompress', modality: ModalityType,
    inputPath: string, outputPath: string, algorithm: AlgorithmType,
    dimensions?: { width: number; height: number; compressionStrength: number }
  ): Promise<CompressionMetrics> {
    const started = performance.now();
    let binary: string;
    let args: string[];
    let audioPcmPath: string | undefined;

    if (modality === 'image' && algorithm === 'image_huffman') {
      binary = config.imageCompressorPath;
      args = [operation, inputPath, outputPath];
    } else if (modality === 'audio' && algorithm === 'audio_huffman') {
      binary = operation === 'compress' ? config.audioCompressorPath : config.audioDecompressorPath;
      if (operation === 'compress' && path.extname(inputPath).toLowerCase() === '.mp3') {
        // Keep the decoded intermediate unique per job so concurrent MP3 jobs
        // cannot overwrite one another; it lets the API report Huffman metrics
        // against the PCM bytes actually encoded.
        audioPcmPath = `${outputPath}.decoded.pcm`;
      }
      args = audioPcmPath ? [inputPath, outputPath, audioPcmPath] : [inputPath, outputPath];
    } else if (modality === 'video' && algorithm === 'video_interframe') {
      binary = operation === 'compress' ? config.videoCompressorPath : config.videoDecompressorPath;
      if (operation === 'compress') {
        if (!dimensions || !Number.isInteger(dimensions.width) || !Number.isInteger(dimensions.height) ||
            dimensions.width <= 0 || dimensions.height <= 0 || dimensions.width % 2 || dimensions.height % 2) {
          throw new Error('Video compression requires positive, even YUV420 width and height values.');
        }
        if (!Number.isInteger(dimensions.compressionStrength) || dimensions.compressionStrength < 3 || dimensions.compressionStrength > 40) {
          throw new Error('Video compression strength must be an integer from 3 to 40.');
        }
        args = [inputPath, outputPath, String(dimensions.width), String(dimensions.height), String(dimensions.compressionStrength)];
      } else args = [inputPath, outputPath];
    } else {
      throw new Error(`Unsupported ${modality} algorithm: ${algorithm}`);
    }

    if (!existsSync(binary)) {
      if (modality === 'video') {
        throw new Error(
          `Video codec is not built: ${binary} was not found. Install OpenCV 4 development files (libopencv-dev and pkg-config), then run ./scripts/build.sh.`,
        );
      }
      throw new Error(`${modality} codec executable was not found: ${binary}. Build the native modules first.`);
    }

    try {
      await execFileAsync(binary, args, {
        cwd: path.dirname(inputPath), timeout: 30 * 60 * 1000, maxBuffer: 8 * 1024 * 1024,
      });
    } catch (error: any) {
      if (audioPcmPath) await rm(audioPcmPath, { force: true }).catch(() => {});
      throw new Error(error.stderr?.trim() || error.message || `${modality} ${operation} failed`);
    }

    try {
      const [input, output, algorithmInput] = await Promise.all([
        stat(inputPath), stat(outputPath), stat(audioPcmPath || inputPath),
      ]);
      const ratio = output.size === 0 ? 0 : algorithmInput.size / output.size;
      return {
        algorithm, modality, originalSizeBytes: input.size,
        algorithmInputSizeBytes: algorithmInput.size,
        compressedSizeBytes: output.size, compressionRatio: ratio,
        spaceSavedPercent: algorithmInput.size === 0 ? 0 : ((algorithmInput.size - output.size) / algorithmInput.size) * 100,
        durationMs: performance.now() - started, success: true,
      };
    } finally {
      if (audioPcmPath) await rm(audioPcmPath, { force: true }).catch(() => {});
    }
  }
}
