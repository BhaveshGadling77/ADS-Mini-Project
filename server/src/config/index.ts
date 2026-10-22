import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

export const config = {
  port: parseInt(process.env.PORT || '3000', 10),
  host: process.env.HOST || '0.0.0.0',
  storageDir: process.env.STORAGE_DIR || path.resolve(__dirname, '../../storage/jobs'),
  engineBinaryPath: process.env.ENGINE_BINARY_PATH || path.resolve(__dirname, '../../../compression-core/build/bin/compression-engine'),
  imageCompressorPath: process.env.IMAGE_COMPRESSOR_PATH || path.resolve(__dirname, '../../../compression-core/modules/image/compressor'),
  audioCompressorPath: process.env.AUDIO_COMPRESSOR_PATH || path.resolve(__dirname, '../../../compression-core/modules/audio/audioCompression'),
  audioDecompressorPath: process.env.AUDIO_DECOMPRESSOR_PATH || path.resolve(__dirname, '../../../compression-core/modules/audio/audioDecompression'),
  videoCompressorPath: process.env.VIDEO_COMPRESSOR_PATH || path.resolve(__dirname, '../../../compression-core/modules/video/videoCompress'),
  videoDecompressorPath: process.env.VIDEO_DECOMPRESSOR_PATH || path.resolve(__dirname, '../../../compression-core/modules/video/videoDecompress'),
  allowedAlgorithms: ['lzw', 'huffman', 'bwt'] as const,
  maxFileSize: parseInt(process.env.MAX_FILE_SIZE_MB || '1024', 10) * 1024 * 1024, // Default 1 GB
};
