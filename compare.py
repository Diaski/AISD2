import subprocess
import time
import sys

def measure_execution(executable, input_file):
    start_time = time.perf_counter()
    
    with open(input_file, 'r') as f_in:
        # Uruchomienie programu i przechwycenie wyjścia
        result = subprocess.run([executable], stdin=f_in, capture_output=True, text=True)
        
    end_time = time.perf_counter()
    
    if result.returncode != 0:
        print(f"BŁĄD: Program {executable} zakończył się kodem {result.returncode}")
        print(result.stderr)
        return None, None
        
    return end_time - start_time, result.stdout

def main():
    if len(sys.argv) != 4:
        print("Użycie: python compare.py <program1> <program2> <plik_wejsciowy>")
        print("Przykład: python compare.py ./prog_stary ./prog_nowy test.in")
        sys.exit(1)
        
    prog1 = sys.argv[1]
    prog2 = sys.argv[2]
    input_file = sys.argv[3]

    print(f"Wczytywanie testu z pliku: {input_file}...\n")
    
    # Mierzenie pierwszego programu
    print(f"Uruchamianie {prog1}...")
    time1, out1 = measure_execution(prog1, input_file)
    if time1 is not None:
        print(f"Czas {prog1}: {time1:.4f} sekund\n")
        
    # Mierzenie drugiego programu
    print(f"Uruchamianie {prog2}...")
    time2, out2 = measure_execution(prog2, input_file)
    if time2 is not None:
        print(f"Czas {prog2}: {time2:.4f} sekund\n")

    # Porównanie wyników
    if out1 is not None and out2 is not None:
        if out1 == out2:
            print("✅ Wyjścia obu programów są IDENTYCZNE.")
        else:
            print("❌ UWAGA: Wyjścia się różnią! (Programy dają różne wyniki)")
            # Zapisz do plików, żeby móc podejrzeć diffa
            with open("out1.txt", "w") as f1, open("out2.txt", "w") as f2:
                f1.write(out1)
                f2.write(out2)
            print("Zapisano logi do out1.txt i out2.txt w celu ręcznego porównania.")

if __name__ == "__main__":
    main()