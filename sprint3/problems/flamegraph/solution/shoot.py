import argparse
import subprocess
import time
import random
import shlex
import os
import signal

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'http://localhost:8080/api/v1/maps/map1',
    'http://localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    return subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run(f'curl -s {ammo}', output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


def run_shell(command):
    subprocess.run(command, shell=True, check=True)


server = run(start_server())
time.sleep(1)
perf_recorder = run(f"perf record -F 99 -p {server.pid} -g -o perf.data")
make_shots()

stop(perf_recorder)
stop(server)
time.sleep(1)

run_shell("perf script -i perf.data | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg")
print('graph.svg created')

