struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)  可用内存的字节数
  uint64 nproc;     // number of process              状态不是unused的进程数
};
