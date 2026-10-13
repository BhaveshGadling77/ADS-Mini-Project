import React, { useState } from 'react';
import { ComparisonResult } from '../types/compression';
import { FileDropzone } from '../components/FileDropzone';
import { compareFile, downloadFile } from '../services/compressionApi';
import { GitCompare, Loader2, Download, Zap, HardDrive, Clock, ArrowDown, Award } from 'lucide-react';

function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

export const Compare: React.FC = () => {
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [result, setResult] = useState<ComparisonResult | null>(null);

  const handleCompare = async () => {
    if (!selectedFile) return;
    setLoading(true);
    setError(null);
    setResult(null);

    try {
      const comp = await compareFile(selectedFile);
      setResult(comp);
    } catch (err: any) {
      setError(err.message || 'Comparison failed');
    } finally {
      setLoading(false);
    }
  };

  const bestRatioItem = result?.results.reduce((prev, curr) =>
    curr.compressionRatio > prev.compressionRatio ? curr : prev
  );

  return (
    <div className="max-w-5xl mx-auto px-4 py-8 sm:py-12 space-y-10">
      <div className="text-center space-y-3">
        <div className="inline-flex items-center space-x-2 px-3 py-1 rounded-full bg-teal-500/10 border border-teal-500/20 text-teal-400 text-xs font-mono font-medium">
          <GitCompare className="w-3.5 h-3.5" />
          <span>Algorithm Comparison & Benchmarking</span>
        </div>
        <h1 className="text-3xl sm:text-4xl font-extrabold tracking-tight text-white">
          COMPARE ALGORITHMS SIDE-BY-SIDE
        </h1>
        <p className="text-base text-slate-400 max-w-xl mx-auto">
          Benchmark <span className="text-emerald-400 font-semibold">LZW</span>, <span className="text-teal-400 font-semibold">Static Huffman</span>, and <span className="text-cyan-400 font-semibold">BWT + MTF</span> on your text in real time.
        </p>
      </div>

      <div className="bg-slate-900/60 border border-slate-800 rounded-3xl p-6 sm:p-8 space-y-6 shadow-2xl backdrop-blur-xl">
        <FileDropzone
          selectedFile={selectedFile}
          onFileSelect={(file) => {
            setSelectedFile(file);
            setResult(null);
            setError(null);
          }}
          mode="compress"
        />

        {error && (
          <div className="p-4 rounded-xl bg-rose-500/10 border border-rose-500/20 text-rose-300 text-sm">
            {error}
          </div>
        )}

        <button
          onClick={handleCompare}
          disabled={!selectedFile || loading}
          className={`w-full py-4 rounded-2xl font-bold text-base flex items-center justify-center space-x-2 transition-all shadow-xl ${
            !selectedFile || loading
              ? 'bg-slate-800 text-slate-500 cursor-not-allowed border border-slate-700/50'
              : 'bg-gradient-to-r from-emerald-500 to-teal-500 hover:from-emerald-400 hover:to-teal-400 text-slate-950 font-extrabold shadow-emerald-500/20'
          }`}
        >
          {loading ? (
            <>
              <Loader2 className="w-5 h-5 animate-spin" />
              <span>Running C++ Benchmark on 3 Algorithms...</span>
            </>
          ) : (
            <>
              <Zap className="w-5 h-5" />
              <span>RUN ALGORITHM COMPARISON</span>
            </>
          )}
        </button>
      </div>

      {result && (
        <div className="space-y-6">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between pb-2 border-b border-slate-800 gap-2">
            <div>
              <h3 className="text-xl font-bold text-white">Benchmark Results</h3>
              <p className="text-xs text-slate-400 font-mono">
                Input: <span className="text-slate-200">{result.originalFileName}</span> ({formatBytes(result.originalSizeBytes)})
              </p>
            </div>
            {bestRatioItem && (
              <div className="inline-flex items-center space-x-2 bg-emerald-500/10 border border-emerald-500/30 text-emerald-400 px-3 py-1.5 rounded-full text-xs font-semibold">
                <Award className="w-4 h-4" />
                <span>Winner: {bestRatioItem.algorithmName} ({bestRatioItem.compressionRatio.toFixed(2)}:1)</span>
              </div>
            )}
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            {result.results.map((item) => {
              const isWinner = bestRatioItem?.algorithm === item.algorithm;
              return (
                <div
                  key={item.algorithm}
                  className={`bg-slate-900 border rounded-2xl p-6 space-y-5 relative transition-all ${
                    isWinner
                      ? 'border-emerald-500 shadow-xl shadow-emerald-500/10'
                      : 'border-slate-800'
                  }`}
                >
                  {isWinner && (
                    <div className="absolute -top-3 right-4 bg-emerald-500 text-slate-950 text-[10px] font-extrabold px-2.5 py-0.5 rounded-full tracking-wider uppercase">
                      Best Ratio
                    </div>
                  )}

                  <div>
                    <h4 className="text-lg font-bold text-white">{item.algorithmName}</h4>
                    <span className="text-[11px] font-mono px-2 py-0.5 rounded bg-slate-800 text-slate-300 border border-slate-700">
                      {item.dataStructure}
                    </span>
                  </div>

                  <div className="space-y-3 pt-2">
                    <div className="flex items-center justify-between text-sm">
                      <span className="text-slate-400 flex items-center space-x-1.5">
                        <HardDrive className="w-4 h-4" />
                        <span>Compressed</span>
                      </span>
                      <span className="font-mono font-bold text-slate-100">
                        {formatBytes(item.compressedSizeBytes)}
                      </span>
                    </div>

                    <div className="flex items-center justify-between text-sm">
                      <span className="text-slate-400 flex items-center space-x-1.5">
                        <ArrowDown className="w-4 h-4 text-emerald-400" />
                        <span>Space Saved</span>
                      </span>
                      <span className="font-mono font-bold text-emerald-400">
                        {item.spaceSavedPercent.toFixed(1)}%
                      </span>
                    </div>

                    <div className="flex items-center justify-between text-sm">
                      <span className="text-slate-400 flex items-center space-x-1.5">
                        <Zap className="w-4 h-4 text-teal-400" />
                        <span>Ratio</span>
                      </span>
                      <span className="font-mono font-bold text-teal-400">
                        {item.compressionRatio.toFixed(2)}:1
                      </span>
                    </div>

                    <div className="flex items-center justify-between text-sm">
                      <span className="text-slate-400 flex items-center space-x-1.5">
                        <Clock className="w-4 h-4 text-cyan-400" />
                        <span>Time</span>
                      </span>
                      <span className="font-mono font-bold text-cyan-400">
                        {item.durationMs.toFixed(2)} ms
                      </span>
                    </div>
                  </div>

                  <button
                    onClick={() => downloadFile(item.downloadUrl, `${result.originalFileName}.${item.algorithm}.adsc`)}
                    className="w-full py-2.5 rounded-xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-slate-200 text-xs font-semibold flex items-center justify-center space-x-2 transition-colors"
                  >
                    <Download className="w-3.5 h-3.5" />
                    <span>Download .adsc</span>
                  </button>
                </div>
              );
            })}
          </div>
        </div>
      )}
    </div>
  );
};
