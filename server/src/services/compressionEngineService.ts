import { execFile } from 'child_process';
import { promisify } from 'util';
import { existsSync } from 'fs';
import { config } from '../config/index.js';
import { AlgorithmType, ModalityType, CompressionMetrics, ComparisonItem, ComparisonResult } from '../types/index.js';

const execFileAsync = promisify(execFile);

export class CompressionEngineService {
  private static verifyBinary(): void {
    if (!existsSync(config.engineBinaryPath)) {
      throw new Error(
        `C++ compression engine binary not found at: ${config.engineBinaryPath}. Please build compression-core first.`
      );
    }
  }

  static async compress(
    inputPath: string,
    outputPath: string,
    algorithm: AlgorithmType,
    modality: ModalityType = 'text'
  ): Promise<CompressionMetrics> {
    this.verifyBinary();

    const args = ['compress', '-a', algorithm, '-m', modality, '-i', inputPath, '-o', outputPath, '--json'];

    try {
      const { stdout, stderr } = await execFileAsync(config.engineBinaryPath, args);
      const cleanStdout = stdout.trim();
      const result = JSON.parse(cleanStdout);

      if (!result.success) {
        throw new Error(result.error || stderr || 'Compression failed');
      }

      return {
        algorithm: result.algorithm,
        modality: result.modality || modality,
        originalSizeBytes: result.originalSizeBytes,
        compressedSizeBytes: result.compressedSizeBytes,
        compressionRatio: result.compressionRatio,
        spaceSavedPercent: result.spaceSavedPercent,
        durationMs: result.durationMs,
        success: true,
      };
    } catch (err: any) {
      if (err.stdout) {
        try {
          const parsed = JSON.parse(err.stdout.trim());
          if (parsed.error) throw new Error(parsed.error);
        } catch (e: any) {
          if (e.message && !e.message.startsWith('Unexpected token')) throw e;
        }
      }
      const msg = err.stderr || err.message || 'Unknown compression engine failure';
      throw new Error(`Engine compression failed: ${msg}`);
    }
  }

  static async decompress(
    inputPath: string,
    outputPath: string,
    algorithm?: AlgorithmType
  ): Promise<CompressionMetrics> {
    this.verifyBinary();

    const args = ['decompress', '-i', inputPath, '-o', outputPath, '--json'];
    if (algorithm) {
      args.push('-a', algorithm);
    }

    try {
      const { stdout, stderr } = await execFileAsync(config.engineBinaryPath, args);
      const cleanStdout = stdout.trim();
      const result = JSON.parse(cleanStdout);

      if (!result.success) {
        throw new Error(result.error || stderr || 'Decompression failed');
      }

      return {
        algorithm: result.algorithm,
        originalSizeBytes: result.originalSizeBytes,
        compressedSizeBytes: result.compressedSizeBytes,
        compressionRatio: result.compressionRatio,
        spaceSavedPercent: 0,
        durationMs: result.durationMs,
        success: true,
      };
    } catch (err: any) {
      const msg = err.stderr || err.message || 'Unknown compression engine failure';
      throw new Error(`Engine decompression failed: ${msg}`);
    }
  }

  static async compare(inputPath: string): Promise<any> {
    this.verifyBinary();

    const args = ['compare', '-i', inputPath, '--json'];

    try {
      const { stdout, stderr } = await execFileAsync(config.engineBinaryPath, args);
      const cleanStdout = stdout.trim();
      return JSON.parse(cleanStdout);
    } catch (err: any) {
      const msg = err.stderr || err.message || 'Unknown compare failure';
      throw new Error(`Engine compare failed: ${msg}`);
    }
  }
}
