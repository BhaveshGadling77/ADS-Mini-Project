import React from 'react';
import { Network, Binary, Shuffle, Code2, Server, Globe, Cpu } from 'lucide-react';

export const About: React.FC = () => {
  return (
    <div className="max-w-5xl mx-auto px-4 py-8 sm:py-12 space-y-12">
      {/* Overview */}
      <div className="space-y-4">
        <h1 className="text-3xl sm:text-4xl font-extrabold text-white">
          About Bitshrink — ADS Mini Project
        </h1>
        <p className="text-base text-slate-400 leading-relaxed max-w-3xl">
          Bitshrink is a high-performance cross-platform multi-modal data compression suite built for the{' '}
          <strong className="text-emerald-400">Advanced Data Structures (ADS) Mini Project</strong>.
          It demonstrates the real-world application of fundamental data structures—including
          Prefix Tries, Binary Trees, Min-Heaps, Quadtrees, Wavelet Trees, and Octrees—across text, image, audio, and video modalities.
        </p>
      </div>

      {/* Architecture Cards */}
      <div className="space-y-4">
        <h2 className="text-xl font-bold text-white flex items-center space-x-2">
          <Server className="w-5 h-5 text-emerald-400" />
          <span>System Architecture</span>
        </h2>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-3">
            <div className="w-10 h-10 rounded-xl bg-blue-500/10 border border-blue-500/20 flex items-center justify-center text-blue-400">
              <Globe className="w-5 h-5" />
            </div>
            <h3 className="font-bold text-slate-100">Web Frontend</h3>
            <p className="text-xs text-slate-400 leading-relaxed">
              React + TypeScript + Vite + Tailwind CSS. Provides a modern interactive dashboard with drag-and-drop uploads, real-time metrics, side-by-side benchmarking, and downloads.
            </p>
          </div>

          <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-3">
            <div className="w-10 h-10 rounded-xl bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center text-emerald-400">
              <Server className="w-5 h-5" />
            </div>
            <h3 className="font-bold text-slate-100">Backend Server</h3>
            <p className="text-xs text-slate-400 leading-relaxed">
              Node.js + TypeScript + Fastify. Validates requests, handles multipart streaming file storage, invokes the compiled C++ executable, and serves REST API responses.
            </p>
          </div>

          <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 space-y-3">
            <div className="w-10 h-10 rounded-xl bg-amber-500/10 border border-amber-500/20 flex items-center justify-center text-amber-400">
              <Cpu className="w-5 h-5" />
            </div>
            <h3 className="font-bold text-slate-100">C++ Compression Core</h3>
            <p className="text-xs text-slate-400 leading-relaxed">
              Native C++17/20 engine built with CMake. Implements custom data structures (Trie, MinHeap, BinaryTree) with bit-level I/O streams and standalone CLI.
            </p>
          </div>
        </div>
      </div>

      {/* Algorithms & Data Structures Deep-Dive */}
      <div className="space-y-6">
        <h2 className="text-xl font-bold text-white flex items-center space-x-2">
          <Code2 className="w-5 h-5 text-teal-400" />
          <span>Text Compression Algorithms & Data Structures</span>
        </h2>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-6 space-y-4">
            <div className="flex items-center space-x-3">
              <div className="p-2.5 rounded-xl bg-emerald-500/10 border border-emerald-500/20 text-emerald-400">
                <Network className="w-6 h-6" />
              </div>
              <div>
                <h3 className="font-bold text-white">LZW Compression</h3>
                <span className="text-[11px] font-mono text-emerald-400">Trie (Prefix Tree)</span>
              </div>
            </div>
            <p className="text-xs text-slate-400 leading-relaxed">
              A dictionary-based compression technique that dynamically constructs a <strong>Prefix Trie</strong> during encoding. Longest recurring substrings are mapped to variable-width integer codes (9 to 16 bits), achieving O(1) character lookup per transition.
            </p>
            <div className="bg-slate-950 p-3 rounded-xl border border-slate-800/80 font-mono text-[11px] text-slate-300 space-y-1">
              <div>Time Complexity: <span className="text-emerald-400">O(N)</span></div>
              <div>Space Complexity: <span className="text-emerald-400">O(D)</span> where D = dict size</div>
            </div>
          </div>

          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-6 space-y-4">
            <div className="flex items-center space-x-3">
              <div className="p-2.5 rounded-xl bg-teal-500/10 border border-teal-500/20 text-teal-400">
                <Binary className="w-6 h-6" />
              </div>
              <div>
                <h3 className="font-bold text-white">Static Huffman</h3>
                <span className="text-[11px] font-mono text-teal-400">Min-Heap + Binary Tree</span>
              </div>
            </div>
            <p className="text-xs text-slate-400 leading-relaxed">
              An entropy coding scheme that assigns variable-length prefix codes based on character frequencies. Built from scratch using a custom <strong>Min-Heap priority queue</strong> to greedily merge nodes into an optimal <strong>Binary Tree</strong>.
            </p>
            <div className="bg-slate-950 p-3 rounded-xl border border-slate-800/80 font-mono text-[11px] text-slate-300 space-y-1">
              <div>Time Complexity: <span className="text-teal-400">O(N + K log K)</span></div>
              <div>Space Complexity: <span className="text-teal-400">O(K)</span> where K = alphabet size</div>
            </div>
          </div>

          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-6 space-y-4">
            <div className="flex items-center space-x-3">
              <div className="p-2.5 rounded-xl bg-cyan-500/10 border border-cyan-500/20 text-cyan-400">
                <Shuffle className="w-6 h-6" />
              </div>
              <div>
                <h3 className="font-bold text-white">BWT + MTF</h3>
                <span className="text-[11px] font-mono text-cyan-400">Suffix Array + MTF List</span>
              </div>
            </div>
            <p className="text-xs text-slate-400 leading-relaxed">
              The <strong>Burrows-Wheeler Transform</strong> permutes the input via lexicographical rotation sorting to group identical symbols, followed by <strong>Move-To-Front (MTF)</strong> list transformations and Zero Run-Length Encoding.
            </p>
            <div className="bg-slate-950 p-3 rounded-xl border border-slate-800/80 font-mono text-[11px] text-slate-300 space-y-1">
              <div>Time Complexity: <span className="text-cyan-400">O(N log N)</span></div>
              <div>Space Complexity: <span className="text-cyan-400">O(N)</span></div>
            </div>
          </div>
        </div>
      </div>

      {/* Team Assignment */}
      <div className="space-y-4">
        <h2 className="text-xl font-bold text-white">Project Team & Module Division</h2>
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-4">
            <div className="text-xs text-slate-500 uppercase font-mono">Text Compression</div>
            <div className="font-bold text-emerald-400 mt-1">Bhavesh</div>
            <div className="text-[11px] text-slate-400 mt-0.5">LZW, Huffman, BWT</div>
          </div>
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-4">
            <div className="text-xs text-slate-500 uppercase font-mono">Image Compression</div>
            <div className="font-bold text-slate-200 mt-1">Jibran</div>
            <div className="text-[11px] text-slate-400 mt-0.5">Quadtree, KD-Tree</div>
          </div>
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-4">
            <div className="text-xs text-slate-500 uppercase font-mono">Audio Compression</div>
            <div className="font-bold text-slate-200 mt-1">Parth</div>
            <div className="text-[11px] text-slate-400 mt-0.5">Wavelet, Delta-RLE</div>
          </div>
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-4">
            <div className="text-xs text-slate-500 uppercase font-mono">Video Compression</div>
            <div className="font-bold text-slate-200 mt-1">Sangram</div>
            <div className="text-[11px] text-slate-400 mt-0.5">Interframe, Octree</div>
          </div>
        </div>
      </div>
    </div>
  );
};
