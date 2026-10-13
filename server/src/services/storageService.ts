import fs from 'fs/promises';
import { existsSync } from 'fs';
import path from 'path';
import crypto from 'crypto';
import { config } from '../config/index.js';
import { JobRecord } from '../types/index.js';

export class StorageService {
  static async init(): Promise<void> {
    if (!existsSync(config.storageDir)) {
      await fs.mkdir(config.storageDir, { recursive: true });
    }
  }

  static createJobId(): string {
    return crypto.randomUUID();
  }

  static getJobDir(jobId: string): string {
    return path.join(config.storageDir, jobId);
  }

  static async initJobDir(jobId: string): Promise<string> {
    const jobDir = this.getJobDir(jobId);
    await fs.mkdir(jobDir, { recursive: true });
    return jobDir;
  }

  static async saveFile(jobDir: string, fileName: string, buffer: Buffer): Promise<string> {
    const filePath = path.join(jobDir, fileName);
    await fs.writeFile(filePath, buffer);
    return filePath;
  }

  static async saveJobRecord(jobRecord: JobRecord): Promise<void> {
    const jobDir = this.getJobDir(jobRecord.jobId);
    const metaPath = path.join(jobDir, 'meta.json');
    await fs.writeFile(metaPath, JSON.stringify(jobRecord, null, 2), 'utf-8');
  }

  static async getJobRecord(jobId: string): Promise<JobRecord | null> {
    const jobDir = this.getJobDir(jobId);
    const metaPath = path.join(jobDir, 'meta.json');
    if (!existsSync(metaPath)) {
      return null;
    }
    const content = await fs.readFile(metaPath, 'utf-8');
    return JSON.parse(content) as JobRecord;
  }

  static async listJobs(limit = 50): Promise<JobRecord[]> {
    await this.init();
    const entries = await fs.readdir(config.storageDir, { withFileTypes: true });
    const records: JobRecord[] = [];

    for (const entry of entries) {
      if (entry.isDirectory()) {
        const metaPath = path.join(config.storageDir, entry.name, 'meta.json');
        if (existsSync(metaPath)) {
          try {
            const raw = await fs.readFile(metaPath, 'utf-8');
            records.push(JSON.parse(raw) as JobRecord);
          } catch {
            // Ignore corrupted record
          }
        }
      }
    }

    records.sort((a, b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime());
    return records.slice(0, limit);
  }
}
