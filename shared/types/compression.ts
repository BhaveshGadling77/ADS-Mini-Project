export type AlgorithmType =
  // Text (Bhavesh)
  | 'lzw'
  | 'huffman'
  | 'bwt'
  // Image (Jibran)
  | 'image_quadtree'
  | 'image_quantization'
  | 'image_wavelet'
  // Audio (Parth)
  | 'audio_wavelet'
  | 'audio_delta_rle'
  | 'audio_lpc'
  // Video (Sangram)
  | 'video_interframe'
  | 'video_octree';

export type ModalityType = 'text' | 'image' | 'audio' | 'video';
export type OperationType = 'compress' | 'decompress' | 'compare';

export interface CompressionMetrics {
  algorithm: string;
  modality?: string;
  originalSizeBytes: number;
  compressedSizeBytes: number;
  compressionRatio: number;
  spaceSavedPercent: number;
  durationMs: number;
  success: boolean;
  errorMessage?: string;
}

export interface JobRecord {
  jobId: string;
  createdAt: string;
  operation: OperationType;
  algorithm: string;
  modality: ModalityType;
  originalFileName: string;
  outputFileName: string;
  metrics: CompressionMetrics;
  downloadUrl: string;
}

export interface ComparisonResult {
  jobId: string;
  originalFileName: string;
  originalSizeBytes: number;
  results: Array<{
    algorithm: AlgorithmType;
    algorithmName: string;
    dataStructure: string;
    compressedSizeBytes: number;
    compressionRatio: number;
    spaceSavedPercent: number;
    durationMs: number;
    success: boolean;
    error?: string;
    downloadUrl: string;
  }>;
}
