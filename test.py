import random

def generate_test_case(filename="test.in", initial_tasks=1000, num_commands=200):
    print(f"Generowanie testu: {initial_tasks} zadań początkowych, {num_commands} komend...")
    
    with open(filename, 'w') as f:
        # 1. Początkowa liczba zadań
        f.write(f"{initial_tasks}\n")
        
        # 2. Generowanie długości początkowych zadań
        tasks = [random.randint(1, 1000) for _ in range(initial_tasks)]
        f.write(" ".join(map(str, tasks)) + "\n")

        current_size = initial_tasks
        next_id = initial_tasks + 1
        active_ids = list(range(1, initial_tasks + 1))

        # Dostępne komendy BEZ 'A'
        commands = ['+', '-', 'B', 'L', 'S', "M"]
        
        for _ in range(num_commands):
            cmd = random.choice(commands)
            
            # Jeśli wylosowano usunięcie, ale nie ma zadań, zmień na dodawanie
            if cmd == '-' and not active_ids:
                cmd = '+'
                
            if cmd == '+':
                # Wstaw na pozycję od 1 do current_size + 1
                k = random.randint(1, current_size + 1)
                p = random.randint(1, 1000)
                f.write(f"+ {k} {p}\n")
                active_ids.append(next_id)
                next_id += 1
                current_size += 1
                
            elif cmd == '-':
                # Usuń istniejące ID
                remove_id = random.choice(active_ids)
                active_ids.remove(remove_id)
                f.write(f"- {remove_id}\n")
                current_size -= 1
                
            else:
                # Szeregowania B, L, S, M - losujemy liczbę maszyn
                m = random.randint(2, 50)
                f.write(f"{cmd} {m}\n")
                
    print(f"Gotowe! Zapisano do pliku {filename}")

if __name__ == "__main__":
    # Możesz zmienić te wartości, jeśli chcesz większy/mniejszy test
    generate_test_case("test.in", initial_tasks=5000, num_commands=1000)