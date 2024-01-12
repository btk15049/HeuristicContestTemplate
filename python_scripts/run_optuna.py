import subprocess
import optuna

import logging
import sys
import tempfile
import pathlib
import os
import util

import run_local

FILE_DIR = pathlib.Path(__file__).parent.absolute()
REPO_ROOT = FILE_DIR.parent.absolute()

TESTCASE_PATH = REPO_ROOT / "data" / "in" / "1-100"

# Add stream handler of stdout to show the messages
optuna.logging.get_logger("optuna").addHandler(logging.StreamHandler(sys.stdout))
study_name = "study0"  # Unique identifier of the study.
storage_name = f"sqlite:///{sys.argv[1]}.db"

storage = optuna.storages.RDBStorage(
    url=storage_name, engine_kwargs={"connect_args": {"timeout": 100}}
)

study = optuna.create_study(
    study_name=study_name,
    storage=storage,
    load_if_exists=True,
    direction="maximize",
)




def run_remote(source_path: pathlib.Path, dir: pathlib.Path, build_cmd: str):
    subprocess.run(
        ["bash", str(REPO_ROOT / "run_jobs.sh")],
        env={**os.environ, **{
            "LOCAL_SOURCE_FILE_PATH": str(source_path),
            "CONTAINER_SOURCE_FILE_PATH": "main.cpp",
            "GCS_INPUT_DIR_PATH": "gs://ahc0xx/in/4000",
            "GCS_OUTPUT_DIR_PATH": "gs://ahc0xx/test/out",  # TODO: change path
            "BUILD_COMMAND": build_cmd,
            "LOCAL_OUTPUT_DIR_PATH": str(dir / "out"),
            "TASKS": "100",
        }},
    ).check_returncode()
    return run_local.parse_scores(dir / "out")


def objective(trial: optuna.Trial):

    params = dict(
        # SIMULATION_MS_THRESHOLD=trial.suggest_float("SIMULATION_MS_THRESHOLD", 5, 10, step=0.5),
        # SIMULATION_SAMPLES_WHEN_FAST_CASE=trial.suggest_int("SIMULATION_SAMPLES_WHEN_FAST_CASE", 100, 200, step=10),
        # SIMULATION_SAMPLES_WHEN_SLOW_CASE=trial.suggest_int("SIMULATION_SAMPLES_WHEN_SLOW_CASE", 80, 150, step=10),
        # SIMULATION_TURNS=trial.suggest_int("SIMULATION_TURNS", 30, 60, step=2),
        # PROBABILITY_SAMPLES=trial.suggest_int("PROBABILITY_SAMPLES", 500, 1500, step=),
        # GREEDY_PICK_DELETE_ONE_THRESHOLD=trial.suggest_float("GREEDY_PICK_DELETE_ONE_THRESHOLD", 0, 1),
        # OVER_THRESHOLD_RATE=trial.suggest_float("OVER_THRESHOLD_RATE", 8.0, 16.0),
        # DELETE_ONE_THRESHOLD_RATE=trial.suggest_float("DELETE_ONE_THRESHOLD_RATE", 0.7, 0.9),
        # SCALE_UP_RATE_A=trial.suggest_float("SCALE_UP_RATE_A", 0.5, 1),
        # SCALE_UP_FREQ_A=trial.suggest_int("SCALE_UP_FREQ_A", 5, 10),
        # SCALE_UP_RATE_B=trial.suggest_float("SCALE_UP_RATE_B", 0.3, 1),
        # SCALE_UP_FREQ_B=trial.suggest_int("SCALE_UP_FREQ_B", 3, 10),
        # SCALE_UP_RATE_C=trial.suggest_float("SCALE_UP_RATE_C", 0.1, 1),
        # C1=trial.suggest_float("C1", 0.1, 10),
        UCB_C=trial.suggest_float("UCB_C", 0.3, 1.0, step= 0.05),
        # EACH_FIRST_TRIES = trial.suggest_int("EACH_FIRST_TRIES", 20, 50, step=5),
    )



    dirname = tempfile.mkdtemp(suffix=study_name)

    dir = pathlib.Path(dirname)
    source_path = REPO_ROOT / "build" / "submit.cpp"
    

    (dir / "out").mkdir(exist_ok=True)

    if (os.environ.get("REMOTE")):
        build_cmd = util.generate_build_command("main.cpp", params)
        average = run_remote(source_path, dir, build_cmd)

    else:
        with tempfile.TemporaryDirectory(prefix=f"trial{trial.number}") as tmpdirname:
            bin = tmpdirname + "/a.out"
            build_cmd = util.generate_build_command(source_path, params, bin)
            # build
            print(build_cmd)
            subprocess.run(
                build_cmd, shell=True
            ).check_returncode()

            # prepare dir
            (dir / "out").mkdir(exist_ok=True)
            (dir / "log").mkdir(exist_ok=True)

            # run
            results = run_local.execute_all(
                 ["./official_tools/target/release/tester", bin],
                TESTCASE_PATH,
                dir / "out",
                dir / "log",
                timeout=15,
                parallelism=10,
            )

            scores = run_local.parse_scores([result.log_file for result in results])
            average = sum(scores) / len(scores)


    return average;


study.optimize(
    objective,
    n_trials=10,
)
