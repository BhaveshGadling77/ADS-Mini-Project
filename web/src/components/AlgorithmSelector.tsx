import React from 'react';
import { AlgorithmType, ModalityType } from '../types/compression';
import { Network, Binary, Shuffle, Music, Video, Layers } from 'lucide-react';

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
        id: 'image_huffman',
        title: 'Huffman Image Coding',
        tagline: 'Lossless coding of BMP pixel bytes',
        dataStructure: 'Min-Heap + Binary Tree',
        assignee: 'Jibran',
        status: 'active',
        icon: <Layers className="w-5 h-5 text-yellow-400" />,
      },
    ],
    audio: [
      {
        id: 'audio_huffman',
        title: 'Huffman Audio Coding',
        tagline: 'Huffman coding for MP3 or raw PCM (s16le, 44.1 kHz, stereo default)',
        dataStructure: 'Min-Heap + Binary Tree',
        assignee: 'Parth',
        status: 'active',
        icon: <Music className="w-5 h-5 text-violet-400" />,
      },
    ],
    video: [
      {
        id: 'video_interframe',
        title: 'Inter-frame Quadtree',
        tagline: 'Motion Delta Block Quadtree',
        dataStructure: 'Quadtree',
        assignee: 'Sangram',
        status: 'active',
        icon: <Video className="w-5 h-5 text-rose-400" />,
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
          {options.some(option => option.status === 'active') ? 'Available in this project build' : 'Prototype'}
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
