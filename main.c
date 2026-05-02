#include <float.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct
{
    int    id;
    double len;
} Task;

typedef struct
{
    Task *data;
    int   size;
    int   capacity;
} TaskList;

typedef struct
{
    int    id;
    double time;
    char   type;
} Fragment;

TaskList tasklist_init(int capacity)
{
    TaskList list;
    list.size     = 0;
    list.capacity = capacity;
    list.data     = (Task *)calloc(capacity, sizeof(Task));
    return list;
}

void tasklist_insert(TaskList *list, int pos, int id, double len)
{
    if (list->capacity == list->size)
    {
        list->data = (Task *)realloc(list->data, list->capacity * 2 * sizeof(Task));
        list->capacity *= 2;
    }
    memmove(&list->data[pos + 1], &list->data[pos], (list->size - pos) * sizeof(Task));
    list->data[pos].id  = id;
    list->data[pos].len = len;
    list->size++;
}

void tasklist_remove_by_id(TaskList *list, int id)
{
    for (int i = 0; i < list->size; i++)
    {
        if (list->data[i].id == id)
        {
            memmove(&list->data[i], &list->data[i + 1], (list->size - i - 1) * sizeof(Task));
            list->size--;
            break;
        }
    }
}

static int find_earliest_machine(const double *end_time, int m)
{
    int best = 0;
    for (int i = 1; i < m; i++)
        if (end_time[i] < end_time[best])
            best = i;
    return best;
}

void print_result(int machine_count, int *task_count, int l_size, Task *assigned_tasks,
                  double *completion, double *machine_end)
{
    double cmax = 0.0, sigma = 0.0;
    for (int i = 0; i < machine_count; i++)
    {
        if (machine_end[i] > cmax)
            cmax = machine_end[i];
        for (int j = 0; j < task_count[i]; j++)
        {
            int id = i * l_size + j;
            sigma += completion[id];
        }
    }
    printf("Cmax: %.0f\n", cmax);
    printf("sigmaC: %g\n", sigma);

    for (int i = 0; i < machine_count; i++)
    {
        printf("M%d:", i + 1);

        for (int j = 0; j < task_count[i]; j++)
        {
            int    index           = i * l_size + j;
            int    id              = assigned_tasks[index].id;
            double completion_time = completion[index];

            printf("( C%d = %.0f )", id, completion_time);
        }
        printf("\n");
    }
}
void schedule_core(Task *tasks, int l_size, int machine_count, int mode)
{
    double *machine_end    = calloc(machine_count, sizeof(double));
    int    *task_count     = calloc(machine_count, sizeof(int));
    Task   *assigned_tasks = calloc((machine_count * l_size), sizeof(Task));
    double *completion     = calloc((machine_count * l_size), sizeof(double));

    if (mode == 0)
    {
        for (int i = 0; i < l_size; i++)
        {
            int machine = find_earliest_machine(machine_end, machine_count);

            double finish        = machine_end[machine] + tasks[i].len;
            machine_end[machine] = finish;

            int pos = task_count[machine];
            int id  = machine * l_size + pos;

            assigned_tasks[id] = tasks[i];
            completion[id]     = finish;
            task_count[machine]++;
        }
    }
    else
    {
        for (int i = 0; i < l_size; i++)
        {
            int machine = i % machine_count;
            int pos     = task_count[machine];
            int id      = machine * l_size + pos;

            assigned_tasks[id] = tasks[i];
            task_count[machine]++;
        }

        for (int mach = 0; mach < machine_count; mach++)
        {
            int count = task_count[mach];

            for (int i = 0; i < count; i++)
            {
                for (int j = i + 1; j < count; j++)
                {
                    int id_part    = mach * l_size;
                    int prev_id    = id_part + i;
                    int checked_id = id_part + j;

                    if (assigned_tasks[prev_id].len < assigned_tasks[checked_id].len)
                    {
                        continue;
                    }
                    if (assigned_tasks[prev_id].len == assigned_tasks[checked_id].len &&
                        assigned_tasks[prev_id].id < assigned_tasks[checked_id].id)
                    {
                        continue;
                    }
                    Task tmp                   = assigned_tasks[prev_id];
                    assigned_tasks[prev_id]    = assigned_tasks[checked_id];
                    assigned_tasks[checked_id] = tmp;
                }
            }
            double current = 0.0;
            for (int j = 0; j < count; j++)
            {
                current += assigned_tasks[mach * l_size + j].len;
                completion[mach * l_size + j] = current;
            }
            machine_end[mach] = current;
        }
    }

    print_result(machine_count, task_count, l_size, assigned_tasks, completion, machine_end);

    free(machine_end);
    free(task_count);
    free(assigned_tasks);
    free(completion);
}

void manual_quick_sort(Task *arr, int low, int high)
{
    if (low < high)
    {
        int  mid   = low + (high - low) / 2;
        Task pivot = arr[mid];

        int i = low;
        int j = high;

        while (i <= j)
        {
            while ((arr[i].len > pivot.len) || (arr[i].len == pivot.len && arr[i].id < pivot.id))
            {
                i++;
            }

            while ((arr[j].len < pivot.len) || (arr[j].len == pivot.len && arr[j].id > pivot.id))
            {
                j--;
            }

            if (i <= j)
            {
                Task tmp = arr[i];
                arr[i]   = arr[j];
                arr[j]   = tmp;
                i++;
                j--;
            }
        }

        if (low < j)
            manual_quick_sort(arr, low, j);
        if (i < high)
            manual_quick_sort(arr, i, high);
    }
}

void sort_tasks_descending_by_len(Task *arr, int l_size)
{
    if (l_size > 1)
    {
        manual_quick_sort(arr, 0, l_size - 1);
    }
}

void schedule_list_scheduling(TaskList *list, int machine_count)
{
    schedule_core(list->data, list->size, machine_count, 0);
}

void schedule_longest_processing_time_first(TaskList *list, int machine_count)
{
    int   l_size = list->size;
    Task *sorted = calloc(l_size, sizeof(Task));

    memcpy(sorted, list->data, l_size * sizeof(Task));
    sort_tasks_descending_by_len(sorted, l_size);
    schedule_core(sorted, l_size, machine_count, 0);

    free(sorted);
}

void schedule_shortest_processing_time_first(TaskList *list, int machine_count)
{
    int l_size = list->size;

    Task *sorted = calloc(l_size, sizeof(Task));
    memcpy(sorted, list->data, l_size * sizeof(Task));
    sort_tasks_descending_by_len(sorted, l_size);
    schedule_core(sorted, l_size, machine_count, 1);

    free(sorted);
}
void b(TaskList *list, int machine_count)
{
    schedule_core(list->data, list->size, machine_count, 0);
}

void l(TaskList *list, int machine_count)
{
    int   l_size     = list->size;
    Task *temp_tasks = calloc(l_size, sizeof(Task));
    Task  temp_task;

    memcpy(temp_tasks, list->data, machine_count * sizeof(Task));
    for (int i = 0; i < l_size - 1; i++)
    {
        int best = i;
        for (int j = i + 1; j < l_size; j++)
        {
            if (temp_tasks[j].len > temp_tasks[best].len)
            {
                best = j;
            }
            else if (temp_tasks[j].len == temp_tasks[best].len &&
                     temp_tasks[j].id > temp_tasks[best].id)
            {
                best = j;
            }
        }
        temp_task        = temp_tasks[i];
        temp_tasks[i]    = temp_tasks[best];
        temp_tasks[best] = temp_task;
    }
    schedule_core(temp_tasks, l_size, machine_count, 0);
    free(temp_tasks);
}

void s(TaskList *list, int machine_count)
{
    int   l_size     = list->size;
    Task *temp_tasks = calloc(l_size, sizeof(Task));
    memcpy(temp_tasks, list->data, l_size * sizeof(Task));

    for (int i = 0; i < l_size - 1; i++)
    {
        int best = i;
        for (int j = i + 1; j < l_size; j++)
        {
            if (temp_tasks[j].len > temp_tasks[best].len)
            {
                best = j;
            }
            else if (temp_tasks[j].len == temp_tasks[best].len &&
                     temp_tasks[j].id > temp_tasks[best].id)
            {
                best = j;
            }
        }
        Task tmp         = temp_tasks[i];
        temp_tasks[i]    = temp_tasks[best];
        temp_tasks[best] = tmp;
    }

    schedule_core(temp_tasks, l_size, machine_count, 1);
    free(temp_tasks);
}

void m(const TaskList *list, int machine_count)
{
    int    l_size  = list->size;
    double sum_len = 0.0;
    double max_len = 0.0;

    for (int i = 0; i < l_size; i++)
    {
        sum_len += list->data[i].len;
        if (list->data[i].len > max_len)
            max_len = list->data[i].len;
    }

    double cmax = sum_len / machine_count;
    if (max_len > cmax)
        cmax = max_len;

    Fragment **sched      = calloc(machine_count, sizeof(Fragment *));
    int       *task_count = calloc(machine_count, sizeof(int));

    for (int i = 0; i < machine_count; i++)
    {
        sched[i] = calloc(l_size + machine_count, sizeof(Fragment));
    }

    int max_id = 0;
    for (int i = 0; i < l_size; i++)
        if (list->data[i].id > max_id)
            max_id = list->data[i].id;
    double *completion_times = calloc(max_id + 1, sizeof(double));

    int    cur_m    = 0;
    double cur_time = 0.0;

    for (int i = 0; i < l_size; i++)
    {
        Task   t         = list->data[i];
        double remaining = t.len;
        int    was_split = 0;
        double eps       = 1e-9;
        while (remaining > eps)
        {
            if (cur_m >= machine_count)
                break;

            double space = cmax - cur_time;

            if (space <= eps)
            {
                cur_m++;
                cur_time = 0;
                continue;
            }

            if (remaining <= space)
            {
                cur_time += remaining;

                char label = was_split ? 'P' : 'C';

                if (task_count[cur_m] < (l_size + machine_count))
                {
                    sched[cur_m][task_count[cur_m]].time = cur_time;
                    sched[cur_m][task_count[cur_m]].type = label;
                    sched[cur_m][task_count[cur_m]].id   = t.id;
                    task_count[cur_m]++;
                }

                if (!was_split)
                {
                    completion_times[t.id] = cur_time;
                }

                remaining = 0.0;
            }
            else
            {
                if (task_count[cur_m] < (l_size + machine_count))
                {
                    sched[cur_m][task_count[cur_m]].time = cmax;
                    sched[cur_m][task_count[cur_m]].type = 'C';
                    sched[cur_m][task_count[cur_m]].id   = t.id;
                    task_count[cur_m]++;
                }

                completion_times[t.id] = cmax;

                remaining -= space;
                cur_m++;
                cur_time  = 0;
                was_split = 1;
            }
        }
    }

    double sigmaC = 0;
    for (int i = 0; i < l_size; i++)
    {
        int task_id = list->data[i].id;
        sigmaC += completion_times[task_id];
    }

    printf("Cmax: %g\n", cmax);
    printf("sigmaC: %g\n", sigmaC);

    for (int i = 0; i < machine_count; i++)
    {
        printf("M%d:", i + 1);
        for (int j = 0; j < task_count[i]; j++)
        {
            printf("( %c%d = %g )", sched[i][j].type, sched[i][j].id, sched[i][j].time);
        }
        printf("\n");
    }

    for (int i = 0; i < machine_count; i++)
        free(sched[i]);
    free(sched);
    free(task_count);
    free(completion_times);
}

void find_optimal_cmax(Task *tasks, int list_size, int task_idx, int machine_count,
                       double *machine_end, double *best_cmax)
{

    for (int i = 0; i < machine_count; i++)
    {
        if (machine_end[i] >= *best_cmax)
        {
            return;
        }
    }

    if (task_idx == list_size)
    {
        double current_cmax = 0.0;
        for (int i = 0; i < machine_count; i++)
        {
            if (machine_end[i] > current_cmax)
            {
                current_cmax = machine_end[i];
            }
        }

        if (current_cmax < *best_cmax)
        {
            *best_cmax = current_cmax;
        }
        return;
    }

    for (int i = 0; i < machine_count; i++)
    {
        machine_end[i] += tasks[task_idx].len;
        find_optimal_cmax(tasks, list_size, task_idx + 1, machine_count, machine_end, best_cmax);
        machine_end[i] -= tasks[task_idx].len;
    }
}

void a(const TaskList *list, int machine_count)
{
    double  best_cmax   = DBL_MAX;
    double *machine_end = calloc(machine_count, sizeof(double));
    find_optimal_cmax(list->data, list->size, 0, machine_count, machine_end, &best_cmax);
    printf("Copt: %g\n", best_cmax);
    free(machine_end);
}

int main()
{
    int n = 0;
    if (scanf(" %d", &n) == EOF)
        return 0;

    TaskList tasks   = tasklist_init(n);
    int      next_id = 1;

    for (int i = 0; i < n; i++)
    {
        double t_len;
        scanf(" %lf", &t_len);
        tasklist_insert(&tasks, i, next_id++, t_len);
    }

    char command;
    while (scanf(" %c", &command) != EOF)
    {
        switch (command)
        {
        case ('+'):
        {
            int k, p;
            scanf(" %d %d", &k, &p);
            tasklist_insert(&tasks, k - 1, next_id++, (double)p);
            break;
        }
        case ('-'):
        {
            int id;
            scanf(" %d", &id);
            tasklist_remove_by_id(&tasks, id);
            break;
        }
        case ('B'):
        {
            int machine_count;
            scanf(" %d", &machine_count);
            schedule_list_scheduling(&tasks, machine_count);
            break;
        }
        case ('L'):
        {
            int machine_count;
            scanf(" %d", &machine_count);
            schedule_longest_processing_time_first(&tasks, machine_count);
            break;
        }
        case ('S'):
        {
            int machine_count;
            scanf(" %d", &machine_count);
            schedule_shortest_processing_time_first(&tasks, machine_count);
            break;
        }
        case ('M'):
        {
            int machine_count;
            scanf(" %d", &machine_count);
            m(&tasks, machine_count);
            break;
        }
        case ('A'):
        {
            int machine_count;
            scanf(" %d", &machine_count);
            a(&tasks, machine_count);
            break;
        }
        }
    }
    free(tasks.data);
    return 0;
}