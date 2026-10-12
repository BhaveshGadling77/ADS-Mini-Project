import React, { useEffect, useState } from 'react';
import { Layers, Activity, History, Info, GitCompare, Cpu } from 'lucide-react';
import { checkServerHealth } from '../services/compressionApi';

interface NavbarProps {
  activeTab: 'workspace' | 'compare' | 'history' | 'about';
  setActiveTab: (tab: 'workspace' | 'compare' | 'history' | 'about') => void;
}

export const Navbar: React.FC<NavbarProps> = ({ activeTab, setActiveTab }) => {
  const [isOnline, setIsOnline] = useState<boolean | null>(null);

  useEffect(() => {
    let mounted = true;
    const check = async () => {
      const ok = await checkServerHealth();
      if (mounted) setIsOnline(ok);
    };
    check();
    const timer = setInterval(check, 10000);
    return () => {
      mounted = false;
      clearInterval(timer);
    };
  }, []);

  return (
    <header className="border-b border-slate-800 bg-slate-900/80 backdrop-blur-md sticky top-0 z-50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 h-16 flex items-center justify-between">
        {/* Brand / Logo */}
        <div className="flex items-center space-x-3 cursor-pointer" onClick={() => setActiveTab('workspace')}>
          <div className="w-10 h-10 rounded-xl bg-gradient-to-tr from-emerald-600 to-teal-400 flex items-center justify-center shadow-lg shadow-emerald-500/20">
            <Cpu className="w-5 h-5 text-white" />
          </div>
          <div>
            <div className="flex items-center space-x-2">
              <span className="font-extrabold text-xl tracking-tight text-white">Bitshrink</span>
              <span className="text-[10px] font-mono px-1.5 py-0.5 rounded bg-emerald-500/10 text-emerald-400 border border-emerald-500/20">
                C++20
              </span>
            </div>
            <p className="text-[11px] text-slate-400 font-medium">ADS Compression Engine</p>
          </div>
        </div>

        {/* Navigation Tabs */}
        <nav className="flex items-center space-x-1 sm:space-x-2">
          <button
            onClick={() => setActiveTab('workspace')}
            className={`flex items-center space-x-2 px-3.5 py-2 rounded-lg text-sm font-medium transition-all ${
              activeTab === 'workspace'
                ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/30 shadow-sm'
                : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/50'
            }`}
          >
            <Layers className="w-4 h-4" />
            <span>Workspace</span>
          </button>

          <button
            onClick={() => setActiveTab('compare')}
            className={`flex items-center space-x-2 px-3.5 py-2 rounded-lg text-sm font-medium transition-all ${
              activeTab === 'compare'
                ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/30 shadow-sm'
                : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/50'
            }`}
          >
            <GitCompare className="w-4 h-4" />
            <span>Compare</span>
          </button>

          <button
            onClick={() => setActiveTab('history')}
            className={`flex items-center space-x-2 px-3.5 py-2 rounded-lg text-sm font-medium transition-all ${
              activeTab === 'history'
                ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/30 shadow-sm'
                : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/50'
            }`}
          >
            <History className="w-4 h-4" />
            <span>History</span>
          </button>

          <button
            onClick={() => setActiveTab('about')}
            className={`flex items-center space-x-2 px-3.5 py-2 rounded-lg text-sm font-medium transition-all ${
              activeTab === 'about'
                ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/30 shadow-sm'
                : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/50'
            }`}
          >
            <Info className="w-4 h-4" />
            <span>About</span>
          </button>
        </nav>

        {/* Server Status Indicator */}
        <div className="hidden md:flex items-center space-x-2 bg-slate-950/60 px-3 py-1.5 rounded-full border border-slate-800 text-xs font-mono">
          <span className="relative flex h-2 w-2">
            {isOnline ? (
              <>
                <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-400 opacity-75"></span>
                <span className="relative inline-flex rounded-full h-2 w-2 bg-emerald-500"></span>
              </>
            ) : (
              <span className="relative inline-flex rounded-full h-2 w-2 bg-rose-500"></span>
            )}
          </span>
          <span className={isOnline ? 'text-slate-300' : 'text-rose-400'}>
            {isOnline ? 'Fastify + C++ Engine' : 'Engine Offline'}
          </span>
        </div>
      </div>
    </header>
  );
};
