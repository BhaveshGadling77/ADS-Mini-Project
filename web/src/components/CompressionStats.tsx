import React from 'react';
import { JobRecord } from '../types/compression';
import { Download, CheckCircle, Clock, Zap, ArrowDown, HardDrive } from 'lucide-react';
import { downloadFile } from '../services/compressionApi';

interface CompressionStatsProps {
  job: JobRecord;
}

function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

export const CompressionStats: React.FC<CompressionStatsProps> = ({ job }) => {
  const { metrics, operation, outputFileName, downloadUrl } = job;
  const isCompress = operation === 'compress';
  const isAudioCompression = isCompress && job.modality === 'audio';
  const isVideoCompression = isCompress && job.modality === 'video';
  const algorithmInputSize = metrics.algorithmInputSizeBytes ?? metrics.originalSizeBytes;
  const sourceSizeChange = metrics.originalSizeBytes > 0
    ? ((metrics.compressedSizeBytes - metrics.originalSizeBytes) / metrics.originalSizeBytes) * 100
    : 0;

  return (
    <div className="w-full bg-slate-900 border border-slate-800 rounded-2xl p-6 sm:p-8 space-y-6 shadow-xl">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between pb-4 border-b border-slate-800 gap-4">
        <div className="flex items-center space-x-3">
          <div className="w-10 h-10 rounded-xl bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center text-emerald-400">
            <CheckCircle className="w-6 h-6" />
          </div>
          <div>
            <h3 className="text-lg font-bold text-white">
              {isCompress ? 'Compression Successful' : 'Decompression Successful'}
            </h3>
            <p className="text-xs text-slate-400 font-mono">
              Algorithm: <span className="text-emerald-400 uppercase font-semibold">{metrics.algorithm}</span> · Processed in {metrics.durationMs.toFixed(2)} ms
            </p>
          </div>
        </div>

        <button
          onClick={() => downloadFile(downloadUrl, outputFileName)}
          className="inline-flex items-center justify-center space-x-2 px-5 py-2.5 rounded-xl bg-gradient-to-r from-emerald-600 to-teal-600 hover:from-emerald-500 hover:to-teal-500 text-white font-semibold text-sm shadow-lg shadow-emerald-500/20 transition-all hover:scale-[1.02] active:scale-[0.98]"
        >
          <Download className="w-4 h-4" />
          <span>Download {isCompress ? 'Compressed File' : 'Restored File'}</span>
        </button>
      </div>

      {isVideoCompression && metrics.compressedSizeBytes > metrics.originalSizeBytes && (
        <div className="rounded-xl border border-amber-500/30 bg-amber-500/10 px-4 py-3 text-sm text-amber-200">
          This setting made the archive {sourceSizeChange.toFixed(1)}% larger. Recompress with a higher strength to trade more detail for a smaller file.
        </div>
      )}

      {/* Metrics Grid */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
          <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
            <HardDrive className="w-3.5 h-3.5" />
            <span>{isAudioCompression ? 'Uploaded File Size' : 'Original Size'}</span>
          </div>
          <div className="text-xl sm:text-2xl font-bold font-mono text-slate-100">
            {formatBytes(metrics.originalSizeBytes)}
          </div>
          <div className="text-[11px] font-mono text-slate-500 mt-0.5">
            {metrics.originalSizeBytes.toLocaleString()} bytes
          </div>
          {isAudioCompression && (
            <div className="text-[11px] font-mono text-slate-500 mt-1">
              Archive is {Math.abs(sourceSizeChange).toFixed(1)}% {sourceSizeChange >= 0 ? 'larger' : 'smaller'} than upload
            </div>
          )}
        </div>

        {isAudioCompression && (
          <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
            <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
              <HardDrive className="w-3.5 h-3.5 text-violet-400" />
              <span>PCM Before Huffman</span>
            </div>
            <div className="text-xl sm:text-2xl font-bold font-mono text-slate-100">
              {formatBytes(algorithmInputSize)}
            </div>
            <div className="text-[11px] font-mono text-slate-500 mt-0.5">
              {algorithmInputSize.toLocaleString()} bytes · codec comparison basis
            </div>
          </div>
        )}

        <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
          <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
            <HardDrive className="w-3.5 h-3.5 text-emerald-400" />
            <span>{isCompress ? 'Compressed Size' : 'Restored Size'}</span>
          </div>
          <div className="text-xl sm:text-2xl font-bold font-mono text-emerald-400">
            {formatBytes(metrics.compressedSizeBytes)}
          </div>
          <div className="text-[11px] font-mono text-slate-500 mt-0.5">
            {metrics.compressedSizeBytes.toLocaleString()} bytes
          </div>
        </div>

        {isCompress ? (
          <>
            <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
              <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
                <ArrowDown className="w-3.5 h-3.5 text-teal-400" />
                <span>Space Saved</span>
              </div>
              <div className="text-xl sm:text-2xl font-bold font-mono text-teal-400">
                {metrics.spaceSavedPercent.toFixed(1)}%
              </div>
              <div className="text-[11px] font-mono text-slate-500 mt-0.5">
                {isAudioCompression ? 'Compared with PCM input' : 'Savings percentage'}
              </div>
            </div>

            <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
              <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
                <Zap className="w-3.5 h-3.5 text-cyan-400" />
                <span>Compression Ratio</span>
              </div>
              <div className="text-xl sm:text-2xl font-bold font-mono text-cyan-400">
                {metrics.compressionRatio.toFixed(2)}:1
              </div>
              <div className="text-[11px] font-mono text-slate-500 mt-0.5">
                {isAudioCompression ? 'PCM input : archive factor' : 'Input : Output factor'}
              </div>
            </div>
          </>
        ) : (
          <>
            <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
              <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
                <CheckCircle className="w-3.5 h-3.5 text-teal-400" />
                <span>Data Integrity</span>
              </div>
              <div className="text-xl sm:text-2xl font-bold font-mono text-teal-400">
                {job.modality === 'video' ? 'Lossy prototype' : job.modality === 'audio' ? 'PCM Huffman lossless' : 'Lossless codec'}
              </div>
              <div className="text-[11px] font-mono text-slate-500 mt-0.5">
                {job.modality === 'video'
                  ? 'Reconstructed YUV may differ from input'
                  : job.modality === 'audio'
                    ? 'MP3 output is re-encoded from decoded PCM'
                    : 'Original data is reconstructed exactly'}
              </div>
            </div>

            <div className="bg-slate-950/60 border border-slate-800/80 rounded-xl p-4">
              <div className="flex items-center space-x-2 text-slate-400 text-xs mb-1">
                <Clock className="w-3.5 h-3.5 text-cyan-400" />
                <span>Decompression Time</span>
              </div>
              <div className="text-xl sm:text-2xl font-bold font-mono text-cyan-400">
                {metrics.durationMs.toFixed(2)} ms
              </div>
              <div className="text-[11px] font-mono text-slate-500 mt-0.5">
                High-speed engine
              </div>
            </div>
          </>
        )}
      </div>

      {/* Visual Size Bar for compression */}
      {isCompress && (isAudioCompression ? algorithmInputSize : metrics.originalSizeBytes) > 0 && (
        <div className="bg-slate-950/40 p-4 rounded-xl border border-slate-800/60">
          <div className="flex justify-between text-xs text-slate-400 font-mono mb-2">
            <span>Size comparison</span>
            <span>
              {metrics.compressedSizeBytes} bytes vs {isAudioCompression ? algorithmInputSize : metrics.originalSizeBytes} bytes
            </span>
          </div>
          <div className="w-full bg-slate-800 h-3 rounded-full overflow-hidden flex">
            <div
              className="bg-emerald-500 h-full rounded-full transition-all duration-500"
              style={{
                width: `${Math.min(100, Math.max(5, (metrics.compressedSizeBytes / (isAudioCompression ? algorithmInputSize : metrics.originalSizeBytes)) * 100))}%`,
              }}
            />
          </div>
        </div>
      )}
    </div>
  );
};
