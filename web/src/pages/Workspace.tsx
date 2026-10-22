import React, { useState } from 'react';
import { AlgorithmType, ModalityType, JobRecord } from '../types/compression';
import { FileDropzone } from '../components/FileDropzone';
import { AlgorithmSelector } from '../components/AlgorithmSelector';
import { CompressionStats } from '../components/CompressionStats';
import { compressFile, decompressFile } from '../services/compressionApi';
import {
  ArrowRight,
  Loader2,
  AlertCircle,
  Sparkles,
  RefreshCw,
  FileText,
  Image as ImageIcon,
  Music,
  Video as VideoIcon,
} from 'lucide-react';

const DEFAULT_ALGO_BY_MODALITY: Record<ModalityType, AlgorithmType> = {
  text: 'lzw',
  image: 'image_huffman',
  audio: 'audio_huffman',
  video: 'video_interframe',
};

export const Workspace: React.FC = () => {
  const [mode, setMode] = useState<'compress' | 'decompress'>('compress');
  const [modality, setModality] = useState<ModalityType>('text');
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [selectedAlgorithm, setSelectedAlgorithm] = useState<AlgorithmType>('lzw');
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [result, setResult] = useState<JobRecord | null>(null);
  const [videoWidth, setVideoWidth] = useState('640');
  const [videoHeight, setVideoHeight] = useState('480');

  const handleModeChange = (newMode: 'compress' | 'decompress') => {
    setMode(newMode);
    setSelectedFile(null);
    setResult(null);
    setError(null);
  };

  const handleModalityChange = (newModality: ModalityType) => {
    setModality(newModality);
    setSelectedAlgorithm(DEFAULT_ALGO_BY_MODALITY[newModality]);
    setSelectedFile(null);
    setResult(null);
    setError(null);
  };

  const handleProcess = async () => {
    if (!selectedFile) return;

    setLoading(true);
    setError(null);
    setResult(null);

    try {
      if (mode === 'compress') {
        const dimensions = modality === 'video'
          ? { width: Number(videoWidth), height: Number(videoHeight) }
          : undefined;
        const job = await compressFile(selectedFile, selectedAlgorithm, modality, dimensions);
        setResult(job);
      } else {
        const job = await decompressFile(selectedFile, selectedAlgorithm, modality);
        setResult(job);
      }
    } catch (err: any) {
      setError(err.message || 'Operation failed. Please verify file integrity.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="max-w-4xl mx-auto px-4 py-8 sm:py-12 space-y-10">
      {/* Hero Section */}
      <div className="text-center space-y-3">
        <div className="inline-flex items-center space-x-2 px-3 py-1 rounded-full bg-emerald-500/10 border border-emerald-500/20 text-emerald-400 text-xs font-mono font-medium">
          <Sparkles className="w-3.5 h-3.5" />
          <span>Bitshrink · ADS Mini Project Compression Suite</span>
        </div>
        <h1 className="text-3xl sm:text-5xl font-extrabold tracking-tight text-white">
          FILE COMPRESSION ENGINE
        </h1>
        <p className="text-base sm:text-lg text-slate-400 max-w-xl mx-auto">
          <span className="text-slate-200 font-medium">Compress less. Understand more.</span> Explore data compression through fundamental algorithms and data structures.
        </p>
      </div>

      {/* Main Interactive Card */}
      <div className="bg-slate-900/60 border border-slate-800 rounded-3xl p-6 sm:p-8 space-y-6 shadow-2xl backdrop-blur-xl">
        {/* Modality Selector Pills */}
        <div className="space-y-2">
          <label className="text-xs font-semibold uppercase tracking-wider text-slate-400 block text-center">
            Select Data Modality
          </label>
          <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 max-w-xl mx-auto">
            <button
              type="button"
              onClick={() => handleModalityChange('text')}
              className={`flex items-center justify-center space-x-2 py-2.5 px-3 rounded-xl font-medium text-xs border transition-all ${
                modality === 'text'
                  ? 'bg-emerald-500/15 border-emerald-500 text-emerald-300 shadow-md shadow-emerald-500/10'
                  : 'bg-slate-950/60 border-slate-800 text-slate-400 hover:text-slate-200 hover:border-slate-700'
              }`}
            >
              <FileText className="w-4 h-4 text-emerald-400" />
              <span>Text</span>
              <span className="text-[10px] opacity-75 font-mono">(Bhavesh)</span>
            </button>
            <button
              type="button"
              onClick={() => handleModalityChange('image')}
              className={`flex items-center justify-center space-x-2 py-2.5 px-3 rounded-xl font-medium text-xs border transition-all ${
                modality === 'image'
                  ? 'bg-amber-500/15 border-amber-500 text-amber-300 shadow-md shadow-amber-500/10'
                  : 'bg-slate-950/60 border-slate-800 text-slate-400 hover:text-slate-200 hover:border-slate-700'
              }`}
            >
              <ImageIcon className="w-4 h-4 text-amber-400" />
              <span>Image</span>
              <span className="text-[10px] opacity-75 font-mono">(Jibran)</span>
            </button>
            <button
              type="button"
              onClick={() => handleModalityChange('audio')}
              className={`flex items-center justify-center space-x-2 py-2.5 px-3 rounded-xl font-medium text-xs border transition-all ${
                modality === 'audio'
                  ? 'bg-violet-500/15 border-violet-500 text-violet-300 shadow-md shadow-violet-500/10'
                  : 'bg-slate-950/60 border-slate-800 text-slate-400 hover:text-slate-200 hover:border-slate-700'
              }`}
            >
              <Music className="w-4 h-4 text-violet-400" />
              <span>Audio</span>
              <span className="text-[10px] opacity-75 font-mono">(Parth)</span>
            </button>
            <button
              type="button"
              onClick={() => handleModalityChange('video')}
              className={`flex items-center justify-center space-x-2 py-2.5 px-3 rounded-xl font-medium text-xs border transition-all ${
                modality === 'video'
                  ? 'bg-rose-500/15 border-rose-500 text-rose-300 shadow-md shadow-rose-500/10'
                  : 'bg-slate-950/60 border-slate-800 text-slate-400 hover:text-slate-200 hover:border-slate-700'
              }`}
            >
              <VideoIcon className="w-4 h-4 text-rose-400" />
              <span>Video</span>
              <span className="text-[10px] opacity-75 font-mono">(Sangram)</span>
            </button>
          </div>
        </div>
        {/* Compress / Decompress Tabs */}
        <div className="flex p-1.5 bg-slate-950/80 rounded-2xl border border-slate-800/80 max-w-md mx-auto">
          <button
            onClick={() => handleModeChange('compress')}
            className={`flex-1 py-2.5 rounded-xl font-semibold text-sm transition-all ${
              mode === 'compress'
                ? 'bg-emerald-600 text-white shadow-lg shadow-emerald-600/30'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            Compress
          </button>
          <button
            onClick={() => handleModeChange('decompress')}
            className={`flex-1 py-2.5 rounded-xl font-semibold text-sm transition-all ${
              mode === 'decompress'
                ? 'bg-emerald-600 text-white shadow-lg shadow-emerald-600/30'
                : 'text-slate-400 hover:text-slate-200'
            }`}
          >
            Decompress
          </button>
        </div>

        {/* File Dropzone */}
        <div className="space-y-2">
          <label className="text-xs font-semibold uppercase tracking-wider text-slate-400">
            {mode === 'compress' ? `Select Your ${modality.toUpperCase()} File` : `Select Compressed ${modality.toUpperCase()} Archive`}
          </label>
          <FileDropzone
            selectedFile={selectedFile}
            onFileSelect={(file) => {
              setSelectedFile(file);
              setResult(null);
              setError(null);
            }}
            mode={mode}
            modality={modality}
          />
        </div>

        {mode === 'compress' && modality === 'video' && (
          <div className="grid grid-cols-2 gap-3 max-w-sm mx-auto">
            <label className="text-xs text-slate-400">YUV420 width
              <input type="number" min="2" step="2" value={videoWidth} onChange={e => setVideoWidth(e.target.value)} className="mt-1 w-full rounded-lg bg-slate-950 border border-slate-700 px-3 py-2 text-slate-100" />
            </label>
            <label className="text-xs text-slate-400">YUV420 height
              <input type="number" min="2" step="2" value={videoHeight} onChange={e => setVideoHeight(e.target.value)} className="mt-1 w-full rounded-lg bg-slate-950 border border-slate-700 px-3 py-2 text-slate-100" />
            </label>
            <p className="col-span-2 text-xs text-slate-500">Video prototype accepts raw YUV420 (.yuv) files. Compression is lossy.</p>
          </div>
        )}

        {/* Algorithm Selector */}
        {mode === 'compress' && (
          <AlgorithmSelector
            modality={modality}
            selectedAlgorithm={selectedAlgorithm}
            onSelectAlgorithm={setSelectedAlgorithm}
            disabled={loading}
          />
        )}

        {/* Error Notification */}
        {error && (
          <div className="p-4 rounded-xl bg-rose-500/10 border border-rose-500/20 text-rose-300 flex items-start space-x-3 text-sm">
            <AlertCircle className="w-5 h-5 flex-shrink-0 text-rose-400 mt-0.5" />
            <div>
              <p className="font-semibold">Operation Error</p>
              <p className="text-xs text-rose-300/80 mt-0.5">{error}</p>
            </div>
          </div>
        )}

        {/* Action Button */}
        <div>
          <button
            onClick={handleProcess}
            disabled={!selectedFile || loading}
            className={`w-full py-4 rounded-2xl font-bold text-base flex items-center justify-center space-x-2 transition-all shadow-xl ${
              !selectedFile || loading
                ? 'bg-slate-800 text-slate-500 cursor-not-allowed border border-slate-700/50'
                : 'bg-emerald-500 hover:bg-emerald-400 text-slate-950 shadow-emerald-500/20 hover:scale-[1.01] active:scale-[0.99]'
            }`}
          >
            {loading ? (
              <>
                <Loader2 className="w-5 h-5 animate-spin" />
                <span>Processing with C++ Engine...</span>
              </>
            ) : mode === 'compress' ? (
              <>
                <span>COMPRESS FILE</span>
                <ArrowRight className="w-5 h-5" />
              </>
            ) : (
              <>
                <span>DECOMPRESS FILE</span>
                <RefreshCw className="w-5 h-5" />
              </>
            )}
          </button>
        </div>
      </div>

      {/* Results Section */}
      {result && <CompressionStats job={result} />}
    </div>
  );
};
