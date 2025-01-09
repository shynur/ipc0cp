要求使用 `g++-10` (及以上) 或 `clang++-16` (及以上).

```bash
make  # 直接测试
make clean  # 清理, 使仓库恢复原样
sudo make  # 以高优先级运行测试进程 以降低轮询的延迟
```

```bash
make proto  # 生成 proto 定义, 以获得 IDE 的提示
```

_________

`.vscode/` 目录下的文件是给我自己用的.
