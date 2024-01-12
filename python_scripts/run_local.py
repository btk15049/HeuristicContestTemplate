import os
import subprocess
from dataclasses import dataclass
import sys
from time import time
from pathlib import Path
from typing import List
from joblib import Parallel, delayed
import math
import util


@dataclass
class ExecuteResult:
    input_file: Path
    output_file: Path
    log_file: Path
    elapsed: float
    message: str

    def is_succeeded(self) -> bool:
        return not bool(self.message)


def execute_command(
    command, input_file, output_file, log_file, timeout=None
) -> ExecuteResult:
    start_time = time()
    message = ""
    try:
        result = subprocess.run(
            command,
            stdin=open(input_file, "r"),
            stdout=open(output_file, "w"),
            stderr=open(log_file, "w"),
            timeout=timeout,
        )
        result.check_returncode()
    except subprocess.TimeoutExpired as e:
        message = f"実行中にタイムアウトが発生しました: {timeout} 秒を超えました。"
    except subprocess.CalledProcessError as e:
        message = f"実行中にエラーが発生しました: コマンドが非ゼロのステータスで終了しました。終了コード: {e.returncode}"
    except Exception as e:
        message = f"実行中にエラーが発生しました: {e}"
    finally:
        end_time = time()
        elapsed = end_time - start_time

    return ExecuteResult(input_file, output_file, log_file, elapsed, message)


def execute_all(
    command, input_dir, output_dir, log_dir, timeout=None, parallelism=1
) -> List[ExecuteResult]:
    input_dir = Path(input_dir)
    output_dir = Path(output_dir)
    log_dir = Path(log_dir)

    output_dir.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)

    input_files = [input_dir / file_name for file_name in os.listdir(input_dir)]

    results = Parallel(n_jobs=parallelism, verbose=10)(
        delayed(execute_command)(
            command,
            input_file,
            output_dir / input_file.name,
            log_dir / input_file.name,
            timeout,
        )
        for input_file in input_files
    )
    results = sorted(results, key=lambda r: r.input_file.name)

    failed_cases = [
        result.input_file.resolve() for result in results if not result.is_succeeded()
    ]
    elapsed_sorted: list[ExecuteResult] = sorted(results, key=lambda x: x.elapsed)
    n = len(elapsed_sorted)

    print(f"実行に失敗したテストケース: {len(failed_cases)}/{len(results)}")
    for failed_case in failed_cases:
        print(f"  {failed_case}")

    print(
        f"実行時間の最大: {elapsed_sorted[-1].elapsed:.2f} 秒, {elapsed_sorted[-1].input_file}"
    )
    print(f"実行時間の上位 5%: {elapsed_sorted[int(n * 0.95)].elapsed:.2f} 秒")
    print(f"実行時間の上位 50%: {elapsed_sorted[int(n * 0.50)].elapsed:.2f} 秒")
    print(f"実行時間の上位 75%: {elapsed_sorted[int(n * 0.25)].elapsed:.2f} 秒")

    return results

def parse_scores(results: List[os.PathLike]) -> List[float]:
    scores = []
    for result in results:
        try:
            score = util.extract_score_from_file(result)
            score = max(score, 1)
        except:
            score = 1
            print(f"{result} のパースに失敗しました。")
        
        scores.append(math.log2(score))
    return scores

if __name__ == "__main__":
    repo_root_path = Path(__file__).resolve().parent.parent
    subprocess.run(util.generate_build_command("src/main.cpp", {}, "./build/bin/a.out"), shell=True).check_returncode()
    results = execute_all(
        ["./official_tools/target/release/tester", "./build/bin/a.out"],
        repo_root_path / "data" / "in" / sys.argv[1],
        repo_root_path / "data" / "out" / sys.argv[1],
        repo_root_path / "data" / "log" / sys.argv[1],
        timeout=60,
        parallelism=10,
    )
    scores = parse_scores([result.log_file for result in results])

    if len(scores) != 0:
        average = sum(scores) / len(scores)
        print(f"Average: {average}")

    print(f"Valid Cases: {len(scores)}")
