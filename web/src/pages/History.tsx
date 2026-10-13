import React, { useEffect, useState } from 'react';
import { JobRecord } from '../types/compression';
import { getJobHistory, downloadFile } from '../services/compressionApi';
import { History as HistoryIcon, Download, RefreshCw, FileText, CheckCircle2, Clock } from 'lucide-react';

function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

export const History: React.FC = () => {
  const [jobs, setJobs] = useState<JobRecord[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  const fetchJobs = async () => {
    setLoading(true);
    try {
      const records = await getJobHistory();
      setJobs(records);
    } catch {
      // Ignored
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchJobs();
  }, []);

  return (
    <div className="max-w-5xl mx-auto px-4 py-8 sm:py-12 space-y-8">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-4 border-b border-slate-800">
        <div>
          <h1 className="text-2xl sm:text-3xl font-extrabold text-white flex items-center space-x-2">
            <HistoryIcon className="w-7 h-7 text-emerald-400" />
            <span>Job History & Records</span>
          </h1>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Previous compression and decompression operations saved on the filesystem.
          </p>
        </div>

        <button
          onClick={fetchJobs}
          disabled={loading}
          className="inline-flex items-center space-x-2 px-4 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 text-slate-200 text-xs font-semibold border border-slate-700 transition-colors"
        >
          <RefreshCw className={`w-3.5 h-3.5 ${loading ? 'animate-spin' : ''}`} />
          <span>Refresh</span>
        </button>
      </div>

      {loading && jobs.length === 0 ? (
        <div className="text-center py-20 text-slate-500 font-mono text-sm">
          Loading history...
        </div>
      ) : jobs.length === 0 ? (
        <div className="text-center py-20 border border-dashed border-slate-800 rounded-3xl bg-slate-900/30">
          <FileText className="w-12 h-12 text-slate-600 mx-auto mb-3" />
          <h3 className="text-base font-semibold text-slate-300">No Operations Yet</h3>
          <p className="text-xs text-slate-500 mt-1">Compress or decompress a file in the workspace to view history here.</p>
        </div>
      ) : (
        <div className="space-y-3">
          {jobs.map((job) => (
            <div
              key={job.jobId}
              className="bg-slate-900 border border-slate-800 hover:border-slate-700 rounded-2xl p-5 flex flex-col sm:flex-row sm:items-center justify-between gap-4 transition-all"
            >
              <div className="flex items-start sm:items-center space-x-4 min-w-0">
                <div className="w-10 h-10 rounded-xl bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center text-emerald-400 flex-shrink-0">
                  <CheckCircle2 className="w-5 h-5" />
                </div>
                <div className="min-w-0">
                  <div className="flex items-center space-x-2">
                    <span className="font-bold text-slate-200 text-sm truncate">{job.originalFileName}</span>
                    <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-slate-800 text-slate-300 uppercase">
                      {job.algorithm}
                    </span>
                    <span className={`text-[10px] font-mono px-2 py-0.5 rounded ${
                      job.operation === 'compress' ? 'bg-emerald-500/10 text-emerald-400' : 'bg-teal-500/10 text-teal-400'
                    }`}>
                      {job.operation}
                    </span>
                  </div>
                  <div className="flex items-center space-x-3 text-xs text-slate-400 font-mono mt-1">
                    <span>{formatBytes(job.metrics.originalSizeBytes)} → {formatBytes(job.metrics.compressedSizeBytes)}</span>
                    <span>·</span>
                    <span>{job.metrics.compressionRatio.toFixed(2)}:1</span>
                    <span>·</span>
                    <span className="flex items-center space-x-1 text-slate-500">
                      <Clock className="w-3 h-3" />
                      <span>{new Date(job.createdAt).toLocaleTimeString()}</span>
                    </span>
                  </div>
                </div>
              </div>

              <button
                onClick={() => downloadFile(job.downloadUrl, job.outputFileName)}
                className="inline-flex items-center space-x-2 px-4 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-slate-200 text-xs font-semibold self-end sm:self-center transition-colors"
              >
                <Download className="w-3.5 h-3.5" />
                <span>Download</span>
              </button>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};
