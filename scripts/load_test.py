import subprocess
import time
import requests
import concurrent.futures
import threading
import psutil
import os
import statistics
import matplotlib.pyplot as plt
from pathlib import Path

# ----------------------------
# CONFIGURABLE PARAMETERS
# ----------------------------
PROJECT_ROOT = Path(__file__).resolve().parent.parent
SERVER_BINARY = PROJECT_ROOT / "build" / "src" / "mini_web_server"  # path to server executable
PORT = 8080
URL = f"http://127.0.0.1:{PORT}/"
DURATION = 10        # total seconds to run the test
CONCURRENCY = 50     # number of simultaneous clients
REQUEST_DELAY = 0.05 # delay between requests per client
LOG_FILE = "load_results.csv"
GRAPH_FILE = "load_latency.png"


# ----------------------------
# HELPER FUNCTIONS
# ----------------------------
def start_server():
    """Start the C++ web server in background."""
    if not SERVER_BINARY.exists():
        raise FileNotFoundError(f"Server binary not found: {SERVER_BINARY}")
    print("[INFO] Starting server...")
    proc = subprocess.Popen([str(SERVER_BINARY), str(PORT)],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    time.sleep(1)  # give server time to start
    return proc


def stop_server(proc):
    """Stop the server process gracefully."""
    print("[INFO] Stopping server...")
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()


def send_request(session, results, errors):
    """Send a single request and record latency."""
    try:
        start = time.time()
        r = session.get(URL, timeout=2)  # avoid hanging
        latency = (time.time() - start) * 1000  # ms
        if r.status_code == 200:
            results.append(latency)
        else:
            errors.append((r.status_code, latency))
    except Exception as e:
        errors.append((str(e), 0))


def worker(stop_event, results, errors):
    """Thread worker that keeps sending requests until stop_event is set."""
    with requests.Session() as session:
        while not stop_event.is_set():
            send_request(session, results, errors)
            time.sleep(REQUEST_DELAY)


def monitor_resources(proc, cpu_usage, mem_usage, stop_event):
    """Periodically sample CPU and memory usage of the server."""
    ps_proc = psutil.Process(proc.pid)
    while not stop_event.is_set():
        try:
            cpu_usage.append(ps_proc.cpu_percent(interval=1))
            mem_usage.append(ps_proc.memory_info().rss / 1024 / 1024)
        except psutil.NoSuchProcess:
            break


def summarize_results(results, errors, cpu_usage, mem_usage):
    """Print summary, save CSV, and plot latency graph."""
    print("\n=== Load Test Summary ===")
    total = len(results) + len(errors)
    print(f"Total Requests: {total}")
    print(f"Successful: {len(results)}")
    print(f"Failed: {len(errors)} ({(len(errors)/total)*100:.2f}%)")
    if results:
        print(f"Average Latency: {statistics.mean(results):.2f} ms")
        print(f"Median Latency: {statistics.median(results):.2f} ms")
        print(f"Max Latency: {max(results):.2f} ms")
        print(f"Min Latency: {min(results):.2f} ms")
    if cpu_usage:
        print(f"Average CPU: {statistics.mean(cpu_usage):.2f}%")
        print(f"Average Memory: {statistics.mean(mem_usage):.2f} MB")

    # Save to CSV
    with open(LOG_FILE, "w") as f:
        f.write("latency_ms\n")
        for l in results:
            f.write(f"{l}\n")
    print(f"\nSaved CSV log to {LOG_FILE}")

    # Plot latency graph
    if results:
        plt.figure(figsize=(8, 4))
        plt.plot(results, label="Request latency (ms)")
        plt.xlabel("Request #")
        plt.ylabel("Latency (ms)")
        plt.title("Web Server Load Test")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(GRAPH_FILE)
        print(f"Saved latency graph to {GRAPH_FILE}")


# ----------------------------
# MAIN EXECUTION
# ----------------------------
if __name__ == "__main__":
    server_proc = start_server()
    stop_event = threading.Event()

    try:
        results, errors = [], []
        cpu_usage, mem_usage = [], []

        # Start resource monitor
        monitor_thread = threading.Thread(target=monitor_resources, args=(server_proc, cpu_usage, mem_usage, stop_event))
        monitor_thread.start()

        # Launch client threads
        print(f"[INFO] Starting {CONCURRENCY} clients for {DURATION}s...")
        with concurrent.futures.ThreadPoolExecutor(max_workers=CONCURRENCY) as executor:
            futures = [executor.submit(worker, stop_event, results, errors) for _ in range(CONCURRENCY)]
            time.sleep(DURATION)
            stop_event.set()
            concurrent.futures.wait(futures)

        monitor_thread.join()
        summarize_results(results, errors, cpu_usage, mem_usage)

    finally:
        stop_server(server_proc)
