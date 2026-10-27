import { FastifyRequest, FastifyReply } from 'fastify';
import path from 'path';
import { existsSync, createReadStream, createWriteStream } from 'fs';
import fs from 'fs/promises';
import { pipeline } from 'stream/promises';
import { StorageService } from '../services/storageService.js';
import { CompressionEngineService } from '../services/compressionEngineService.js';
import { ModalityCompressionService } from '../services/modalityCompressionService.js';
import { AlgorithmType, JobRecord, ModalityType, ComparisonItem } from '../types/index.js';
import { config } from '../config/index.js';

export class CompressionController {
  static async compress(req: FastifyRequest, reply: FastifyReply) {
    const parts = req.parts();
    const jobId = StorageService.createJobId();
    const jobDir = await StorageService.initJobDir(jobId);

    let fileName = 'file.txt';
    let algorithm: AlgorithmType = 'lzw';
    let modality: ModalityType = 'text';
    let inputPath = '';
    let fileReceived = false;
    let videoWidth = 0;
    let videoHeight = 0;
    let videoCompressionStrength = 12;

    for await (const part of parts) {
      if (part.type === 'file') {
        fileName = path.basename(part.filename || 'file.txt');
        const inputFileName = `input${path.extname(fileName) || '.bin'}`;
        inputPath = path.join(jobDir, inputFileName);

        await pipeline(part.file, createWriteStream(inputPath));

        if (part.file.truncated) {
          await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
          return reply.status(413).send({
            success: false,
            error: `File exceeds maximum upload limit of ${config.maxFileSize / (1024 * 1024)} MB`,
          });
        }
        fileReceived = true;
      } else {
        if (part.fieldname === 'algorithm' && part.value) {
          algorithm = (part.value as string).toLowerCase() as AlgorithmType;
        } else if (part.fieldname === 'modality' && part.value) {
          modality = (part.value as string).toLowerCase() as ModalityType;
        } else if (part.fieldname === 'videoWidth') {
          videoWidth = Number(part.value);
        } else if (part.fieldname === 'videoHeight') {
          videoHeight = Number(part.value);
        } else if (part.fieldname === 'videoCompressionStrength') {
          videoCompressionStrength = Number(part.value);
        }
      }
    }

    if (!fileReceived || !inputPath) {
      return reply.status(400).send({ error: 'No file uploaded' });
    }

    // The browser's `accept` attribute is only a picker hint (drag-and-drop
    // bypasses it), so enforce the formats supported by each prototype here.
    const allowedInput: Record<ModalityType, { extensions: string[]; description: string }> = {
      text: { extensions: [], description: '' },
      image: { extensions: ['.bmp'], description: '24-bit uncompressed BMP image' },
      audio: { extensions: ['.mp3', '.pcm'], description: 'MP3 or raw PCM audio' },
      video: { extensions: ['.yuv'], description: 'raw YUV420 video' },
    };
    const expected = allowedInput[modality];
    if (!expected) {
      await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
      return reply.status(400).send({ error: `Unsupported modality: ${modality}` });
    }
    if (modality !== 'text' && !expected.extensions.includes(path.extname(fileName).toLowerCase())) {
      await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
      const extensions = expected.extensions.join(' or ');
      return reply.status(400).send({
        error: `${modality[0].toUpperCase()}${modality.slice(1)} compression requires ${expected.description} (${extensions}) input.`,
      });
    }
    if (modality === 'video') {
      const frameBytes = videoWidth * videoHeight * 3 / 2;
      const { size } = await fs.stat(inputPath);
      if (!Number.isSafeInteger(frameBytes) || frameBytes <= 0 || size === 0 || size % frameBytes !== 0 ||
          !Number.isInteger(videoCompressionStrength) || videoCompressionStrength < 3 || videoCompressionStrength > 40) {
        await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
        return reply.status(400).send({
          error: 'Video settings are invalid. Verify the YUV420 dimensions, upload complete raw frames, and set compression strength from 3 to 40.',
        });
      }
    }
    if (modality === 'audio' && path.extname(fileName).toLowerCase() === '.pcm') {
      const { size } = await fs.stat(inputPath);
      // Default PCM is signed 16-bit stereo: 2 bytes x 2 channels per frame.
      if (size === 0 || size % 4 !== 0) {
        await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
        return reply.status(400).send({
          error: 'PCM input must contain complete signed 16-bit stereo frames (4 bytes per frame). Expected 44.1 kHz, little-endian PCM.',
        });
      }
    }

    const outputExtension = { text: '.adsc', image: '.huf2', audio: '.huf', video: '.cmp' }[modality];
    const outputFileName = `${path.basename(fileName, path.extname(fileName))}${outputExtension}`;
    const outputPath = path.join(jobDir, outputFileName);

    try {
      const metrics = modality === 'text'
        ? await CompressionEngineService.compress(inputPath, outputPath, algorithm, modality)
        : await ModalityCompressionService.run('compress', modality, inputPath, outputPath, algorithm,
          modality === 'video' ? { width: videoWidth, height: videoHeight, compressionStrength: videoCompressionStrength } : undefined);

      const jobRecord: JobRecord = {
        jobId,
        createdAt: new Date().toISOString(),
        operation: 'compress',
        algorithm,
        modality,
        originalFileName: fileName,
        outputFileName,
        inputPath,
        outputPath,
        metrics,
        downloadUrl: `/api/jobs/${jobId}/download`,
      };

      await StorageService.saveJobRecord(jobRecord);
      return reply.send({ success: true, ...jobRecord });
    } catch (err: any) {
      return reply.status(500).send({
        success: false,
        error: err.message || 'Compression failed',
      });
    }
  }

  static async decompress(req: FastifyRequest, reply: FastifyReply) {
    const parts = req.parts();
    const jobId = StorageService.createJobId();
    const jobDir = await StorageService.initJobDir(jobId);

    let fileName = 'compressed.adsc';
    let algorithm: AlgorithmType | undefined = undefined;
    let modality: ModalityType = 'text';
    let inputPath = '';
    let fileReceived = false;

    for await (const part of parts) {
      if (part.type === 'file') {
        fileName = path.basename(part.filename || 'compressed.adsc');
        const inputFileName = `input${path.extname(fileName) || '.bin'}`;
        inputPath = path.join(jobDir, inputFileName);

        await pipeline(part.file, createWriteStream(inputPath));

        if (part.file.truncated) {
          await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
          return reply.status(413).send({
            success: false,
            error: `File exceeds maximum upload limit of ${config.maxFileSize / (1024 * 1024)} MB`,
          });
        }
        fileReceived = true;
      } else if (part.fieldname === 'algorithm' && part.value) {
        algorithm = (part.value as string).toLowerCase() as AlgorithmType;
      } else if (part.fieldname === 'modality' && part.value) {
        modality = (part.value as string).toLowerCase() as ModalityType;
      }
    }

    if (!fileReceived || !inputPath) {
      return reply.status(400).send({ error: 'No file uploaded' });
    }

    const extensions: Record<ModalityType, string> = { text: '.adsc', image: '.huf2', audio: '.huf', video: '.cmp' };
    const restoredExtensions: Record<ModalityType, string> = { text: '.txt', image: '.bmp', audio: '.mp3', video: '.yuv' };
    const compressedExtension = extensions[modality];
    if (!fileName.toLowerCase().endsWith(compressedExtension)) {
      return reply.status(400).send({ error: `Expected a ${compressedExtension} ${modality} archive` });
    }
    let outputFileName = `${path.basename(fileName, compressedExtension)}${restoredExtensions[modality]}`;

    const outputPath = path.join(jobDir, outputFileName);

    try {
      algorithm = algorithm || ({ text: undefined, image: 'image_huffman', audio: 'audio_huffman', video: 'video_interframe' } as const)[modality];
      const metrics = modality === 'text'
        ? await CompressionEngineService.decompress(inputPath, outputPath, algorithm)
        : await ModalityCompressionService.run('decompress', modality, inputPath, outputPath, algorithm!);

      const jobRecord: JobRecord = {
        jobId,
        createdAt: new Date().toISOString(),
        operation: 'decompress',
        algorithm: metrics.algorithm,
        modality,
        originalFileName: fileName,
        outputFileName,
        inputPath,
        outputPath,
        metrics,
        downloadUrl: `/api/jobs/${jobId}/download`,
      };

      await StorageService.saveJobRecord(jobRecord);
      return reply.send({ success: true, ...jobRecord });
    } catch (err: any) {
      return reply.status(500).send({
        success: false,
        error: err.message || 'Decompression failed',
      });
    }
  }

  static async compare(req: FastifyRequest, reply: FastifyReply) {
    const parts = req.parts();
    const jobId = StorageService.createJobId();
    const jobDir = await StorageService.initJobDir(jobId);

    let fileName = 'file.txt';
    let inputPath = '';
    let fileReceived = false;

    for await (const part of parts) {
      if (part.type === 'file') {
        fileName = part.filename || 'file.txt';
        const inputFileName = `input_${fileName}`;
        inputPath = path.join(jobDir, inputFileName);

        await pipeline(part.file, createWriteStream(inputPath));

        if (part.file.truncated) {
          await fs.rm(jobDir, { recursive: true, force: true }).catch(() => {});
          return reply.status(413).send({
            success: false,
            error: `File exceeds maximum upload limit of ${config.maxFileSize / (1024 * 1024)} MB`,
          });
        }
        fileReceived = true;
      }
    }

    if (!fileReceived || !inputPath) {
      return reply.status(400).send({ error: 'No file uploaded' });
    }

    const stat = await fs.stat(inputPath);
    const originalSizeBytes = stat.size;

    const algos: Array<{ id: AlgorithmType; name: string; ds: string }> = [
      { id: 'lzw', name: 'LZW', ds: 'Trie (Prefix Tree)' },
      { id: 'huffman', name: 'Static Huffman', ds: 'Min-Heap + Binary Tree' },
      { id: 'bwt', name: 'BWT + MTF', ds: 'Suffix Array + Move-to-Front List' },
    ];

    const results: ComparisonItem[] = [];

    for (const algo of algos) {
      const outName = `${fileName}.${algo.id}.adsc`;
      const outPath = path.join(jobDir, outName);
      try {
        const metrics = await CompressionEngineService.compress(inputPath, outPath, algo.id);
        results.push({
          algorithm: algo.id,
          algorithmName: algo.name,
          dataStructure: algo.ds,
          compressedSizeBytes: metrics.compressedSizeBytes,
          compressionRatio: metrics.compressionRatio,
          spaceSavedPercent: metrics.spaceSavedPercent,
          durationMs: metrics.durationMs,
          success: true,
          downloadUrl: `/api/jobs/${jobId}/download?file=${encodeURIComponent(outName)}`,
        });
      } catch (err: any) {
        results.push({
          algorithm: algo.id,
          algorithmName: algo.name,
          dataStructure: algo.ds,
          compressedSizeBytes: 0,
          compressionRatio: 0,
          spaceSavedPercent: 0,
          durationMs: 0,
          success: false,
          error: err.message,
          downloadUrl: '',
        });
      }
    }

    return reply.send({
      jobId,
      originalFileName: fileName,
      originalSizeBytes,
      results,
    });
  }

  static async getJob(req: FastifyRequest<{ Params: { jobId: string } }>, reply: FastifyReply) {
    const { jobId } = req.params;
    const record = await StorageService.getJobRecord(jobId);
    if (!record) {
      return reply.status(404).send({ error: 'Job not found' });
    }
    return reply.send(record);
  }

  static async downloadJob(
    req: FastifyRequest<{ Params: { jobId: string }; Querystring: { file?: string } }>,
    reply: FastifyReply
  ) {
    const { jobId } = req.params;
    const { file: customFile } = req.query;

    const jobDir = StorageService.getJobDir(jobId);
    let targetPath: string;
    let downloadName: string;

    if (customFile) {
      targetPath = path.join(jobDir, customFile);
      downloadName = customFile;
    } else {
      const record = await StorageService.getJobRecord(jobId);
      if (!record || !existsSync(record.outputPath)) {
        return reply.status(404).send({ error: 'File not found for this job' });
      }
      targetPath = record.outputPath;
      downloadName = record.outputFileName;
    }

    if (!existsSync(targetPath)) {
      return reply.status(404).send({ error: 'Requested file not found on disk' });
    }

    reply.header('Content-Disposition', `attachment; filename="${downloadName}"`);
    reply.header('Content-Type', 'application/octet-stream');
    return reply.send(createReadStream(targetPath));
  }

  static async getHistory(
    req: FastifyRequest<{ Querystring: { limit?: string } }>,
    reply: FastifyReply
  ) {
    const limit = parseInt(req.query.limit || '50', 10);
    const jobs = await StorageService.listJobs(limit);
    return reply.send({ jobs });
  }
}
