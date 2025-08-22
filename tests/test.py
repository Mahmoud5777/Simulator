import subprocess
import threading
import queue
import time

# ---------------------------
# Configuration
# ---------------------------
SIMULATOR_EXECUTABLE = "/home/mahmoud/Bureau/Simulator/build/runSimulator"
TEST_TIMEOUT = 5.0  # timeout global pour chaque message

TEST_MESSAGES = {
    "single_1byte": "A",
    "single_3bytes": "ABC",
    "single_max": "1234567",
    "multi_min": "12345678",
    "multi_medium": "Hello this is a medium length message for CAN TP testing",
    "multi_500": "A" * 500
}

# Couleurs ANSI
COLOR_GREEN = "\033[92m"
COLOR_RED = "\033[91m"
COLOR_CYAN = "\033[96m"
COLOR_RESET = "\033[0m"

# ---------------------------
# Tester Class
# ---------------------------
class CANTPTester:
    def __init__(self, simulator_path):
        self.simulator_path = simulator_path
        self.results = {}

    def run_simulator_command(self, commands, timeout=TEST_TIMEOUT):
        """Exécute le simulateur et lit les logs jusqu'au timeout, silencieux"""
        try:
            proc = subprocess.Popen(
                [self.simulator_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )

            output_queue = queue.Queue()

            def reader(pipe, queue):
                for line in pipe:
                    queue.put(line)

            threading.Thread(target=reader, args=(proc.stdout, output_queue), daemon=True).start()

            # Envoyer les commandes
            for cmd in commands:
                proc.stdin.write(cmd + "\n")
                proc.stdin.flush()
                time.sleep(0.05)

            # Lire les logs avec timeout
            logs = []
            start_time = time.time()
            while time.time() - start_time < timeout:
                try:
                    line = output_queue.get_nowait()
                    logs.append(line.strip())
                    start_time = time.time()  # reset timeout à chaque nouvelle ligne
                except queue.Empty:
                    time.sleep(0.01)

            try:
                proc.stdin.write("0\n")  # sortir proprement
                proc.stdin.flush()
            except:
                pass
            proc.terminate()
            proc.wait(timeout=1.0)
            return logs

        except Exception as e:
            return [f"ERROR: {str(e)}"]

    def analyze_logs(self, logs, original_bytes, expected_type):
        """Analyse les logs et compare les octets envoyés avec l'original"""
        sent_data = []

        for line in logs:
            line_lower = line.strip().lower()
            if "single frame" in line_lower:
                # Single Frame -> ignorer 1er octet (PCI)
                bytes_str = line.split(":")[-1].strip().split()
                data_bytes = [int(b, 16) for b in bytes_str[1:]]
                sent_data.extend(data_bytes)
            elif "first frame" in line_lower:
                # First Frame -> ignorer 2 octets (PCI + longueur)
                bytes_str = line.split(":")[-1].strip().split()
                data_bytes = [int(b, 16) for b in bytes_str[2:]]
                sent_data.extend(data_bytes)
            elif "consecutive frame" in line_lower:
                # Consecutive Frame -> ignorer 1er octet (num séquence)
                bytes_str = line.split(":")[-1].strip().split()
                data_bytes = [int(b, 16) for b in bytes_str[1:]]
                sent_data.extend(data_bytes)

        received_len = len(sent_data)
        expected_len = len(original_bytes)

        if sent_data == list(original_bytes):
            status = f"{COLOR_GREEN}[PASS]{COLOR_RESET}"
            comment = f"Transmission correcte ({received_len} octets)."
        else:
            status = f"{COLOR_RED}[FAIL]{COLOR_RESET}"
            comment = f"{expected_type} incomplet/mal transmis ({received_len}/{expected_len} octets)."

        return status, comment

    def test_message(self, test_name, message):
        print(f"{COLOR_CYAN}→ Test en cours : {test_name} ({len(message)} bytes){COLOR_RESET}")
        commands = [
            "1",      # envoyer message
            message,
            "0"       # retour menu
        ]
        logs = self.run_simulator_command(commands)
        expected_type = "Single Frame" if test_name.startswith("single") else "Multi-Frame"
        original_bytes = [ord(c) for c in message]
        status, comment = self.analyze_logs(logs, original_bytes, expected_type)
        self.results[test_name] = f"{status} : {comment}"

    def run_tests(self):
        print(f"\n{'='*70}\nLANCEMENT DES TESTS CAN TP\n{'='*70}\n")
        # Single Frames
        for key in ["single_1byte", "single_3bytes", "single_max"]:
            self.test_message(key, TEST_MESSAGES[key])
        # Multi Frames
        for key in ["multi_min", "multi_medium", "multi_500"]:
            self.test_message(key, TEST_MESSAGES[key])

        # Résumé final
        print(f"\n{'='*70}")
        print(f"{'Test':<15} | {'Résultat'}")
        print(f"{'-'*70}")
        for key, res in self.results.items():
            print(f"{key:<15} | {res}")
        print(f"{'='*70}\n")

# ---------------------------
# Exécution
# ---------------------------
if __name__ == "__main__":
    tester = CANTPTester(SIMULATOR_EXECUTABLE)
    tester.run_tests()

