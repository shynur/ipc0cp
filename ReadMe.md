## 拉取

```bash
git clone --recurse-submodule http://github.com/shynur/ipc0cp.git
```

## 工具链要求

- 编译器: `g++-10` (及以上) 或 `clang++-16` (及以上).

- Bash.

- `flatc`: FlatBuffers 的翻译器, 将 `example.fbs` 文件翻译成 `example.fbs.hpp`.
  你需要下载正确的 `flatc` 版本, 在 clone 本仓库后执行:

  ```bash
  $ cd lib/flatbuffers
  $ git tag --points-at HEAD
  v24.12.23  # <-- 仅作为示例
  ```

  根据输出的版本号, 到 <https://github.com/google/flatbuffers/releases> 自行下载对应版本,
  给它设置执行权限, 并放到 `$PATH` 下.

## 运行

```bash
make  # 直接测试
make clean  # 清理, 使仓库恢复原样
sudo make  # 以高优先级运行测试进程 以降低轮询的延迟
```

## Misc

```bash
make proto  # 生成 proto 定义, 以获得 IDE 的提示
```

(这一步当然不是必须的.)

___

`.vscode/` 目录下的文件是给我自己用的, 建议删除.
