import subprocess
import time
import threading

# Chemin vers ton simulateur
SIMULATOR_EXECUTABLE = "/home/mahmoud/Bureau/Simulator/build/runSimulator"

def start_receiver_mode(repeat_count=12):
    try:
        # Lancer le simulateur
        proc = subprocess.Popen(
            [SIMULATOR_EXECUTABLE],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        # Thread pour afficher la sortie du simulateur
        def read_output():
            for line in proc.stdout:
                print(line, end='')

        threading.Thread(target=read_output, daemon=True).start()

        time.sleep(0.5)  # attendre que le menu apparaisse

        # Envoyer "2" autant de fois que demandé
        for _ in range(repeat_count):
            proc.stdin.write("2\n")
            proc.stdin.flush()
            time.sleep(0.3)  # petit délai pour que le simulateur traite l'input

        print(f"\nSent '2' {repeat_count} times to enter Receiver mode.")

        # Laisser le simulateur finir son exécution ou attendre l'arrêt manuel
        proc.wait()

    except KeyboardInterrupt:
        print("\nReceiver stopped by user.")
        proc.terminate()
        proc.wait()

if __name__ == "__main__":
    start_receiver_mode(repeat_count=12)

