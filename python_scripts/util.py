from os import PathLike

SCORE_LINE_PREFIX = "Score = "

def extract_score_from_file(file: PathLike):
    with open(file, "r") as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith(SCORE_LINE_PREFIX):
                return int(line[len(SCORE_LINE_PREFIX) :].strip())
    return None

def generate_build_command(source_file: PathLike, params, binary_path: PathLike = "a.out"):
    return f"g++ {str(source_file)} -std=c++23 -O3 -o {str(binary_path)} " + (
        " ".join([f"-DPARAM_{key}={val}" for key, val in params.items()])
    )
