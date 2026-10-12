import React from 'react';
import { AlgorithmType, ModalityType } from '../types/compression';
import { Network, Binary, Shuffle, Image, Music, Video, Grid, Layers } from 'lucide-react';

interface AlgorithmSelectorProps {
  modality: ModalityType;
  selectedAlgorithm: AlgorithmType;
  onSelectAlgorithm: (algo: AlgorithmType) => void;
  disabled?: boolean;
}

interface AlgorithmOption {
  id: AlgorithmType;
  title: string;
  tagline: string;
  dataStructure: string;
  assignee: string;
  status: 'active' | 'in_dev';
  icon: React.ReactNode;
}

export const AlgorithmSelector: React.FC<AlgorithmSelectorProps> = ({
  modality,
  selectedAlgorithm,
  onSelectAlgorithm,
  disabled = false,
}) => {
  const optionsMap: Record<ModalityType, AlgorithmOption[]> = {
    text: [
      {
        id: 'lzw',
        title: 'LZW',
        tagline: 'Dictionary Compression',
        dataStructure: 'Trie (Prefix Tree)',
        assignee: 'Bhavesh',
        status: 'active',
        icon: <Network className="w-5 h-5 text-emerald-400" />,
      },
      {
        id: 'huffman',
        title: 'Static Huffman',
        tagline: 'Entropy Frequency Coding',
        dataStructure: 'Min-Heap + Tree',
        assignee: 'Bhavesh',
        status: 'active',
        icon: <Binary className="w-5 h-5 text-teal-400" />,
      },
      {
        id: 'bwt',
        title: 'BWT + MTF',
        tagline: 'Block Sorting & Transform',
        dataStructure: 'Suffix Array + MTF',
        assignee: 'Bhavesh',
        status: 'active',
        icon: <Shuffle className="w-5 h-5 text-cyan-400" />,
      },
    ],
    image: [
      {
        id: 'image_quadtree',
        title: 'Spatial Quadtree',
        tagline: 'Recursive 2D Decomposition',
        dataStructure: 'Quadtree',
        assignee: 'Jibran',
        status: 'in_dev',
        icon: <Grid className="w-5 h-5 text-amber-400" />,
      },
      {
        id: 'image_quantization',
        title: 'Color Quantization',
        tagline: 'Spatial RGB Palette Cluster',
        dataStructure: 'KD-Tree',
        assignee: 'Jibran',
        status: 'in_dev',
        icon: <Image className="w-5 h-5 text-orange-400" />,
      },
      {
        id: 'image_wavelet',
        title: '2D Haar Wavelets',
        tagline: 'Multi-resolution Transform',
        dataStructure: 'Wavelet Matrix',
        assignee: 'Jibran',
        status: 'in_dev',
        icon: <Layers className="w-5 h-5 text-yellow-400" />,
      },
    ],
    audio: [
      {
        id: 'audio_wavelet',
        title: '1D Wavelet Tree',
        tagline: 'Temporal Frequency Split',
        dataStructure: 'Wavelet Tree',
        assignee: 'Parth',
        status: 'in_dev',
        icon: <Music className="w-5 h-5 text-violet-400" />,
      },
      {
        id: 'audio_delta_rle',
        title: 'Delta + RLE',
        tagline: 'Differential Sample Coding',
        dataStructure: 'Delta Run-Length',
        assignee: 'Parth',
        status: 'in_dev',
        icon: <Shuffle className="w-5 h-5 text-purple-400" />,
      },
      {
        id: 'audio_lpc',
        title: 'LPC + Huffman',
        tagline: 'Linear Predictive Coding',
        dataStructure: 'Predictor + Heap',
        assignee: 'Parth',
        status: 'in_dev',
        icon: <Binary className="w-5 h-5 text-fuchsia-400" />,
      },
    ],
    video: [
      {
        id: 'video_interframe',
        title: 'Inter-frame Quadtree',
        tagline: 'Motion Delta Block Quadtree',
        dataStructure: 'Quadtree',
        assignee: 'Sangram',
        status: 'in_dev',
        icon: <Video className="w-5 h-5 text-rose-400" />,
      },
      {
        id: 'video_octree',
        title: '3D Octree',
        tagline: 'Spatio-Temporal Volume',
        dataStructure: '3D Octree',
        assignee: 'Sangram',
        status: 'in_dev',
        icon: <Grid className="w-5 h-5 text-pink-400" />,
      },
    ],
  };

  const options = optionsMap[modality] || optionsMap.text;

  return (
    <div className="w-full space-y-3">
      <div className="flex items-center justify-between">
        <label className="text-xs font-semibold uppercase tracking-wider text-slate-400">
          Available Algorithms ({modality.toUpperCase()})
        </label>
        <span className="text-[11px] font-mono text-slate-500">
          {modality === 'text' ? 'Implemented by Bhavesh' : `Teammate Module (${options[0]?.assignee})`}
        </span>
      </div>

      <div className="grid grid-cols-1 sm:grid-cols-3 gap-3">
        {options.map((opt) => {
          const isSelected = selectedAlgorithm === opt.id;
          return (
            <div
              key={opt.id}
              onClick={() => !disabled && onSelectAlgorithm(opt.id)}
              className={`relative rounded-xl p-4 cursor-pointer transition-all border ${
                isSelected
                  ? 'bg-slate-900 border-emerald-500 shadow-md shadow-emerald-500/10 ring-1 ring-emerald-500'
                  : 'bg-slate-900/50 border-slate-800 hover:border-slate-700 hover:bg-slate-900/80'
              } ${disabled ? 'opacity-50 cursor-not-allowed' : ''}`}
            >
              <div className="flex items-start justify-between mb-2">
                <div className="p-2 rounded-lg bg-slate-800/80 border border-slate-700">
                  {opt.icon}
                </div>
                <div className="flex flex-col items-end space-y-1">
                  <span
                    className={`text-[10px] font-mono px-2 py-0.5 rounded-full border ${
                      isSelected
                        ? 'bg-emerald-500/10 text-emerald-300 border-emerald-500/30'
                        : 'bg-slate-800 text-slate-400 border-slate-700'
                    }`}
                  >
                    {opt.dataStructure}
                  </span>
                  {opt.status === 'in_dev' && (
                    <span className="text-[9px] font-mono text-amber-400 bg-amber-500/10 border border-amber-500/20 px-1.5 py-0.2 rounded">
                      In Dev · {opt.assignee}
                    </span>
                  )}
                </div>
              </div>

              <h4 className="font-bold text-slate-100 text-sm">{opt.title}</h4>
              <p className="text-xs text-slate-400 mt-0.5">{opt.tagline}</p>
            </div>
          );
        })}
      </div>
    </div>
  );
};
