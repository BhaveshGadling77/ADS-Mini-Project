import { AlgorithmType, JobRecord, ComparisonResult } from '../types/compression';

const API_BASE = import.meta.env.VITE_API_BASE_URL || '';

export async function checkServerHealth(): Promise<boolean> {
  try {
    const res = await fetch(`${API_BASE}/health`);
    return res.ok;
  } catch {
    return false;
  }
}

export async function compressFile(
  file: File,
  algorithm: AlgorithmType = 'lzw',
  modality = 'text',
  videoDimensions?: { width: number; height: number }
): Promise<JobRecord> {
  const formData = new FormData();
  formData.append('file', file);
  formData.append('algorithm', algorithm);
  formData.append('modality', modality);
  if (videoDimensions) {
    formData.append('videoWidth', String(videoDimensions.width));
    formData.append('videoHeight', String(videoDimensions.height));
  }

  const res = await fetch(`${API_BASE}/api/compress`, {
    method: 'POST',
    body: formData,
  });

  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: 'Compression request failed' }));
    throw new Error(err.error || 'Compression request failed');
  }

  return res.json();
}

export async function decompressFile(
  file: File,
  algorithm?: AlgorithmType,
  modality = 'text'
): Promise<JobRecord> {
  const formData = new FormData();
  formData.append('file', file);
  if (algorithm) {
    formData.append('algorithm', algorithm);
  }
  formData.append('modality', modality);

  const res = await fetch(`${API_BASE}/api/decompress`, {
    method: 'POST',
    body: formData,
  });

  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: 'Decompression request failed' }));
    throw new Error(err.error || 'Decompression request failed');
  }

  return res.json();
}

export async function compareFile(file: File): Promise<ComparisonResult> {
  const formData = new FormData();
  formData.append('file', file);

  const res = await fetch(`${API_BASE}/api/compare`, {
    method: 'POST',
    body: formData,
  });

  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: 'Comparison request failed' }));
    throw new Error(err.error || 'Comparison request failed');
  }

  return res.json();
}

export async function getJobHistory(limit = 50): Promise<JobRecord[]> {
  const res = await fetch(`${API_BASE}/api/history?limit=${limit}`);
  if (!res.ok) {
    throw new Error('Failed to fetch job history');
  }
  const data = await res.json();
  return data.jobs || [];
}

export function downloadFile(url: string, filename: string): void {
  const fullUrl = url.startsWith('http') ? url : `${API_BASE}${url}`;
  const a = document.createElement('a');
  a.href = fullUrl;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
}
