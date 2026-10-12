import assert from 'assert';
import { spawn, ChildProcess } from 'child_process';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function sleep(ms: number) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

async function runTests() {
  console.log('--- Starting Backend Server Integration Tests ---');

  const serverProcess: ChildProcess = spawn('node', ['dist/index.js'], {
    cwd: path.resolve(__dirname, '..'),
    env: { ...process.env, PORT: '3001' },
    stdio: 'inherit',
  });

  try {
    // Wait for server to boot
    let serverReady = false;
    for (let i = 0; i < 20; ++i) {
      try {
        const res = await fetch('http://localhost:3001/health');
        if (res.ok) {
          serverReady = true;
          break;
        }
      } catch {
        await sleep(300);
      }
    }

    assert(serverReady, 'Server failed to start');
    console.log('✓ Health check passed');

    // 1. Test POST /api/compress
    const sampleText = 'TOBEORNOTTOBEORTOBEORNOT THIS IS A TEST OF THE ADSC BACKEND ENGINE!';
    const form = new FormData();
    const blob = new Blob([sampleText], { type: 'text/plain' });
    form.append('file', blob, 'sample.txt');
    form.append('algorithm', 'lzw');
    form.append('modality', 'text');

    const compRes = await fetch('http://localhost:3001/api/compress', {
      method: 'POST',
      body: form,
    });
    assert(compRes.ok, `Compress request failed: ${compRes.statusText}`);
    const compData = await compRes.json();
    assert(compData.success, 'Compress response success is not true');
    assert(compData.jobId, 'No jobId returned');
    assert(compData.metrics.compressedSizeBytes > 0, 'Invalid compressed size');
    console.log('✓ Compress API passed (LZW):', compData.metrics);

    const jobId = compData.jobId;

    // 2. Test GET /api/jobs/:jobId
    const jobRes = await fetch(`http://localhost:3001/api/jobs/${jobId}`);
    assert(jobRes.ok, 'Get job failed');
    const jobData = await jobRes.json();
    assert.strictEqual(jobData.jobId, jobId);
    console.log('✓ Job retrieval passed');

    // 3. Test GET /api/jobs/:jobId/download
    const dlRes = await fetch(`http://localhost:3001/api/jobs/${jobId}/download`);
    assert(dlRes.ok, 'Download failed');
    const compBytes = await dlRes.arrayBuffer();
    assert(compBytes.byteLength === compData.metrics.compressedSizeBytes, 'Download size mismatch');
    console.log('✓ Download compressed file passed');

    // 4. Test POST /api/decompress
    const decompForm = new FormData();
    const compBlob = new Blob([compBytes], { type: 'application/octet-stream' });
    decompForm.append('file', compBlob, 'sample.txt.adsc');

    const decompRes = await fetch('http://localhost:3001/api/decompress', {
      method: 'POST',
      body: decompForm,
    });
    assert(decompRes.ok, 'Decompress request failed');
    const decompData = await decompRes.json();
    assert(decompData.success, 'Decompress success is not true');

    const decompDl = await fetch(`http://localhost:3001/api/jobs/${decompData.jobId}/download`);
    const restoredText = await decompDl.text();
    assert.strictEqual(restoredText, sampleText, 'Restored text does not match original text');
    console.log('✓ Decompress and round-trip text restoration verified!');

    // 5. Test POST /api/compare
    const compareForm = new FormData();
    compareForm.append('file', blob, 'sample.txt');
    const compareRes = await fetch('http://localhost:3001/api/compare', {
      method: 'POST',
      body: compareForm,
    });
    assert(compareRes.ok, 'Compare request failed');
    const compareData = await compareRes.json();
    assert.strictEqual(compareData.results.length, 3, 'Expected 3 algorithm results');
    console.log('✓ Algorithm compare API passed (LZW, Huffman, BWT):', compareData.results.map((r: any) => `${r.algorithm}: ${r.spaceSavedPercent}%`));

    // 6. Test GET /api/history
    const histRes = await fetch('http://localhost:3001/api/history');
    assert(histRes.ok, 'History request failed');
    const histData = await histRes.json();
    assert(histData.jobs.length >= 2, 'History jobs missing');
    console.log(`✓ History API passed (${histData.jobs.length} jobs found)`);

    console.log('\n--- ALL BACKEND INTEGRATION TESTS PASSED! ---');
  } finally {
    serverProcess.kill('SIGTERM');
  }
}

runTests().catch((err) => {
  console.error('Test failed:', err);
  process.exit(1);
});
