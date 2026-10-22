import React, { useRef, useState } from 'react';
import { UploadCloud, FileText, X, CheckCircle2 } from 'lucide-react';
import { ModalityType } from '../types/compression';

interface FileDropzoneProps {
  selectedFile: File | null;
  onFileSelect: (file: File | null) => void;
  mode: 'compress' | 'decompress';
  modality?: ModalityType;
}

function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

const MODALITY_ACCEPT: Record<ModalityType, string> = {
  text: '.txt,.md,.json,.csv,.c,.cpp,.h,.hpp,.py,.js,.ts,.html,.xml,.log',
  image: '.bmp',
  audio: '.mp3',
  video: '.yuv',
};

const MODALITY_HINT: Record<ModalityType, string> = {
  text: 'Supports .txt, .md, code, data files (up to 1 GB)',
  image: '24-bit uncompressed BMP only · produces a lossless .huf2 archive',
  audio: 'MP3 input · Huffman-codes decoded PCM into a .huf archive',
  video: 'Raw YUV420 (.yuv) only · lossy prototype · requires dimensions',
};

const MODALITY_ARCHIVE: Record<ModalityType, string> = {
  text: '.adsc', image: '.huf2', audio: '.huf', video: '.cmp',
};

export const FileDropzone: React.FC<FileDropzoneProps> = ({
  selectedFile,
  onFileSelect,
  mode,
  modality = 'text',
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);

  const handleDragOver = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragging(true);
  };

  const handleDragLeave = () => {
    setIsDragging(false);
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragging(false);
    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      onFileSelect(e.dataTransfer.files[0]);
    }
  };

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files && e.target.files.length > 0) {
      onFileSelect(e.target.files[0]);
    }
  };

  const archiveExtension = MODALITY_ARCHIVE[modality];
  const acceptPattern = mode === 'compress' ? MODALITY_ACCEPT[modality] : archiveExtension;
  const hintText = mode === 'compress' ? MODALITY_HINT[modality] : `Supports ${archiveExtension} ${modality} archives`;
  const dropTitle = mode === 'compress' ? `Drop your ${modality} file here` : `Drop your compressed (${archiveExtension}) file here`;

  return (
    <div className="w-full">
      <input
        ref={inputRef}
        type="file"
        className="hidden"
        onChange={handleInputChange}
        accept={acceptPattern}
      />

      {!selectedFile ? (
        <div
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onDrop={handleDrop}
          onClick={() => inputRef.current?.click()}
          className={`border-2 border-dashed rounded-2xl p-8 sm:p-12 text-center cursor-pointer transition-all duration-200 ${
            isDragging
              ? 'border-emerald-500 bg-emerald-500/10 scale-[1.01]'
              : 'border-slate-800 hover:border-slate-700 bg-slate-900/40 hover:bg-slate-900/70'
          }`}
        >
          <div className="w-16 h-16 mx-auto mb-4 rounded-2xl bg-slate-800/80 border border-slate-700 flex items-center justify-center text-slate-300 group-hover:scale-105 transition-transform">
            <UploadCloud className="w-8 h-8 text-emerald-400" />
          </div>

          <h3 className="text-lg font-semibold text-slate-200 mb-1">
            {dropTitle}
          </h3>
          <p className="text-sm text-slate-400 mb-4">
            or <span className="text-emerald-400 font-medium underline underline-offset-4">browse files</span> from your computer
          </p>

          <div className="inline-flex items-center space-x-2 text-xs font-mono text-slate-500 bg-slate-950/60 px-3 py-1.5 rounded-full border border-slate-800">
            <span>{hintText}</span>
          </div>
        </div>
      ) : (
        <div className="border border-slate-800 rounded-2xl p-5 bg-slate-900/70 flex items-center justify-between">
          <div className="flex items-center space-x-4 min-w-0">
            <div className="w-12 h-12 rounded-xl bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center flex-shrink-0">
              <FileText className="w-6 h-6 text-emerald-400" />
            </div>
            <div className="min-w-0">
              <div className="flex items-center space-x-2">
                <span className="font-semibold text-slate-200 truncate">{selectedFile.name}</span>
                <CheckCircle2 className="w-4 h-4 text-emerald-400 flex-shrink-0" />
              </div>
              <p className="text-xs font-mono text-slate-400 mt-0.5">
                Size: <span className="text-slate-300 font-medium">{formatBytes(selectedFile.size)}</span>
              </p>
            </div>
          </div>

          <button
            onClick={() => onFileSelect(null)}
            className="p-2 rounded-lg text-slate-400 hover:text-rose-400 hover:bg-slate-800 transition-colors ml-4 flex-shrink-0"
            title="Remove file"
          >
            <X className="w-5 h-5" />
          </button>
        </div>
      )}
    </div>
  );
};
