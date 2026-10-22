import { execFile } from 'child_process';
import { promisify } from 'util';
import { stat } from 'fs/promises';
import path from 'path';
import { config } from '../config/index.js';
import { AlgorithmType, ModalityType, CompressionMetrics } from '../types/index.js';

const execFileAsync = promisify(execFile);

export class ModalityCompressionService {
  static async run(
    operation: 'compress' | 'decompress', modality: ModalityType,
    inputPath: string, outputPath: string, algorithm: AlgorithmType,
    dimensions?: { width: number; height: number }
  ): Promise<CompressionMetrics> {
    const started = performance.now();
    let binary: string;
    let args: string[];

    if (modality === 'image' && algorithm === 'image_huffman') {
      binary = config.imageCompressorPath;
      args = [operation, inputPath, outputPath];
    } else if (modality === 'audio' && algorithm === 'audio_huffman') {
      binary = operation === 'compress' ? config.audioCompressorPath : config.audioDecompressorPath;
      args = [inputPath, outputPath];
    } else if (modality === 'video' && algorithm === 'video_interframe') {
      binary = operation === 'compress' ? config.videoCompressorPath : config.videoDecompressorPath;
      if (operation === 'compress') {
        if (!dimensions || !Number.isInteger(dimensions.width) || !Number.isInteger(dimensions.height) ||
            dimensions.width <= 0 || dimensions.height <= 0 || dimensions.width % 2 || dimensions.height % 2) {
          throw new Error('Video compression requires positive, even YUV420 width and height values.');
        }
        args = [inputPath, outputPath, String(dimensions.width), String(dimensions.height)];
      } else args = [inputPath, outputPath];
    } else {
      throw new Error(`Unsupported ${modality} algorithm: ${algorithm}`);
    }

    try {
      await execFileAsync(binary, args, {
        cwd: path.dirname(inputPath), timeout: 30 * 60 * 1000, maxBuffer: 8 * 1024 * 1024,
      });
    } catch (error: any) {
      throw new Error(error.stderr?.trim() || error.message || `${modality} ${operation} failed`);
    }

    const [input, output] = await Promise.all([stat(inputPath), stat(outputPath)]);
    const ratio = output.size === 0 ? 0 : input.size / output.size;
    return {
      algorithm, modality, originalSizeBytes: input.size, compressedSizeBytes: output.size,
      compressionRatio: ratio,
      spaceSavedPercent: input.size === 0 ? 0 : ((input.size - output.size) / input.size) * 100,
      durationMs: performance.now() - started, success: true,
    };
  }
}
