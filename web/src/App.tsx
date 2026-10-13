import { useState } from 'react';
import { Navbar } from './components/Navbar';
import { Workspace } from './pages/Workspace';
import { Compare } from './pages/Compare';
import { History } from './pages/History';
import { About } from './pages/About';

export function App() {
  const [activeTab, setActiveTab] = useState<'workspace' | 'compare' | 'history' | 'about'>('workspace');

  return (
    <div className="min-h-screen flex flex-col bg-slate-950 text-slate-100 font-sans selection:bg-emerald-500 selection:text-white">
      <Navbar activeTab={activeTab} setActiveTab={setActiveTab} />

      <main className="flex-1 pb-16">
        {activeTab === 'workspace' && <Workspace />}
        {activeTab === 'compare' && <Compare />}
        {activeTab === 'history' && <History />}
        {activeTab === 'about' && <About />}
      </main>

      <footer className="border-t border-slate-900 bg-slate-950/80 py-6 text-center text-xs text-slate-500 font-mono">
        <div className="max-w-7xl mx-auto px-4 flex flex-col sm:flex-row items-center justify-between gap-2">
          <span>Bitshrink · ADS Mini Project (Advanced Data Structures)</span>
          <span>C++20 Engine · Fastify Backend · React Vite Frontend</span>
        </div>
      </footer>
    </div>
  );
}

export default App;
