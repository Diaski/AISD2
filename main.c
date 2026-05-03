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
    int   size, capacity;
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
    list.data     = calloc(capacity, sizeof(Task));
    list.size     = 0;
    list.capacity = capacity;
    return list;
}

void tasklist_insert(TaskList *list, int pos, int id, double len)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        list->data = realloc(list->data, list->capacity * sizeof(Task));
    }
    memmove(&list->data[pos + 1], &list->data[pos], (list->size - pos) * sizeof(Task));
    list->data[pos].id  = id;
    list->data[pos].len = len;
    list->size++;
}

void tasklist_remove(TaskList *list, int id)
{
    for (int i = 0; i < list->size; i++)
    {
        if (list->data[i].id != id)
            continue;
        memmove(&list->data[i], &list->data[i + 1], (list->size - i - 1) * sizeof(Task));
        list->size--;
        return;
    }
}

static void insertion_sort(Task *arr, int n, int (*cmp)(const void *, const void *))
{
    for (int i = 1; i < n; i++)
    {
        Task key = arr[i];
        int  j   = i - 1;
        while (j >= 0 && cmp(&arr[j], &key) > 0)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static int cmp_desc(const void *a, const void *b)
{
    const Task *ta = a, *tb = b;
    if (ta->len != tb->len)
        return (ta->len < tb->len) ? 1 : -1;
    return tb->id - ta->id;
}

static int cmp_asc(const void *a, const void *b)
{
    const Task *ta = a, *tb = b;
    if (ta->len != tb->len)
        return (ta->len > tb->len) ? 1 : -1;
    return ta->id - tb->id;
}

static Task *sorted_copy(const TaskList *list, int (*cmp)(const void *, const void *))
{
    Task *copy = malloc(list->size * sizeof(Task));
    memcpy(copy, list->data, list->size * sizeof(Task));

    insertion_sort(copy, list->size, cmp);
    return copy;
}

static int earliest_machine(const double *end, int m)
{
    int best = 0;
    for (int i = 1; i < m; i++)
        if (end[i] < end[best])
            best = i;
    return best;
}

static void print_schedule(int m, int *count, int stride, Task *assigned, double *finish,
                           double *end)
{
    double cmax = 0, sigma = 0;
    for (int i = 0; i < m; i++)
    {
        if (end[i] > cmax)
            cmax = end[i];
        for (int j = 0; j < count[i]; j++)
            sigma += finish[i * stride + j];
    }
    printf("Cmax: %.0f\n", cmax);
    printf("sigmaC: %g\n", sigma);
    for (int i = 0; i < m; i++)
    {
        printf("M%d:", i + 1);
        for (int j = 0; j < count[i]; j++)
        {
            int idx = i * stride + j;
            printf("( C%d = %.0f )", assigned[idx].id, finish[idx]);
        }
        printf("\n");
    }
}

static void schedule_core(Task *tasks, int n, int m, int mode)
{
    double *end      = calloc(m, sizeof(double));
    int    *count    = calloc(m, sizeof(int));
    Task   *assigned = calloc(m * n, sizeof(Task));
    double *finish   = calloc(m * n, sizeof(double));

    if (mode == 0)
    {
        for (int i = 0; i < n; i++)
        {
            int mach = earliest_machine(end, m);
            end[mach] += tasks[i].len;
            int idx       = mach * n + count[mach]++;
            assigned[idx] = tasks[i];
            finish[idx]   = end[mach];
        }
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            int mach = i % m;

            assigned[mach * n + count[mach]++] = tasks[i];
        }
        for (int mach = 0; mach < m; mach++)
        {
            insertion_sort(&assigned[mach * n], count[mach], cmp_asc);
            double cur = 0;
            for (int j = 0; j < count[mach]; j++)
            {
                cur += assigned[mach * n + j].len;
                finish[mach * n + j] = cur;
            }
            end[mach] = cur;
        }
    }

    print_schedule(m, count, n, assigned, finish, end);
    free(end);
    free(count);
    free(assigned);
    free(finish);
}

void schedule_ls(const TaskList *list, int m) { schedule_core(list->data, list->size, m, 0); }

void schedule_lpt(const TaskList *list, int m)
{
    Task *sorted = sorted_copy(list, cmp_desc);
    schedule_core(sorted, list->size, m, 0);
    free(sorted);
}

void schedule_spt(const TaskList *list, int m)
{
    Task *sorted = sorted_copy(list, cmp_desc);
    schedule_core(sorted, list->size, m, 1);
    free(sorted);
}

void schedule_mcnaughton(const TaskList *list, int m)
{
    int    n   = list->size;
    double sum = 0, max_len = 0;
    for (int i = 0; i < n; i++)
    {
        sum += list->data[i].len;
        if (list->data[i].len > max_len)
            max_len = list->data[i].len;
    }
    double cmax = (sum / m > max_len) ? sum / m : max_len;

    int       *count = calloc(m, sizeof(int));
    Fragment **sched = calloc(m, sizeof(Fragment *));
    for (int i = 0; i < m; i++)
        sched[i] = calloc(n + m, sizeof(Fragment));

    int max_id = 0;
    for (int i = 0; i < n; i++)
        if (list->data[i].id > max_id)
            max_id = list->data[i].id;
    double *comp = calloc(max_id + 1, sizeof(double));

    int          cur_m = 0;
    double       cur_t = 0;
    const double eps   = 1e-9;

    for (int i = 0; i < n; i++)
    {
        Task   t     = list->data[i];
        double rem   = t.len;
        int    split = 0;

        while (rem > eps)
        {
            if (cur_m >= m)
                break;
            double space = cmax - cur_t;

            if (space <= eps)
            {
                cur_m++;
                cur_t = 0;
                continue;
            }

            Fragment frag;
            if (rem <= space)
            {
                cur_t += rem;

                frag.id   = t.id;
                frag.type = split ? 'P' : 'C';
                frag.time = cur_t;

                sched[cur_m][count[cur_m]++] = frag;
                if (!split)
                    comp[t.id] = cur_t;
                rem = 0;
            }
            else
            {

                frag.id   = t.id;
                frag.time = cmax;
                frag.type = 'C';

                sched[cur_m][count[cur_m]++] = frag;
                comp[t.id]                   = cmax;
                rem -= space;
                cur_m++;
                cur_t = 0;
                split = 1;
            }
        }
    }

    double sigma = 0;
    for (int i = 0; i < n; i++)
        sigma += comp[list->data[i].id];

    printf("Cmax: %g\n", cmax);
    printf("sigmaC: %g\n", sigma);
    for (int i = 0; i < m; i++)
    {
        printf("M%d:", i + 1);
        for (int j = 0; j < count[i]; j++)
            printf("( %c%d = %g )", sched[i][j].type, sched[i][j].id, sched[i][j].time);
        printf("\n");
        free(sched[i]);
    }
    free(sched);
    free(count);
    free(comp);
}

static void find_optimal(Task *tasks, int n, int idx, int m, double *end, double *best)
{
    for (int i = 0; i < m; i++)
        if (end[i] >= *best)
            return;

    if (idx == n)
    {
        double cmax = 0;
        for (int i = 0; i < m; i++)
            if (end[i] > cmax)
                cmax = end[i];
        if (cmax < *best)
            *best = cmax;
        return;
    }
    for (int i = 0; i < m; i++)
    {
        end[i] += tasks[idx].len;
        find_optimal(tasks, n, idx + 1, m, end, best);
        end[i] -= tasks[idx].len;
    }
}

void schedule_optimal(const TaskList *list, int m)
{
    double  best = DBL_MAX;
    double *end  = calloc(m, sizeof(double));
    find_optimal(list->data, list->size, 0, m, end, &best);
    printf("Copt: %g\n", best);
    free(end);
}

int main(void)
{
    int n;
    if (scanf(" %d", &n) == EOF)
        return 0;

    TaskList tasks   = tasklist_init(n);
    int      next_id = 1;

    for (int i = 0; i < n; i++)
    {
        double len;
        scanf(" %lf", &len);
        tasklist_insert(&tasks, i, next_id++, len);
    }

    char cmd;
    while (scanf(" %c", &cmd) != EOF)
    {
        int m;
        switch (cmd)
        {
        case '+':
        {
            int k, p;
            scanf(" %d %d", &k, &p);
            tasklist_insert(&tasks, k - 1, next_id++, (double)p);
            break;
        }
        case '-':
        {
            int id;
            scanf(" %d", &id);
            tasklist_remove(&tasks, id);
            break;
        }
        case 'B':
            scanf(" %d", &m);
            schedule_ls(&tasks, m);
            break;
        case 'L':
            scanf(" %d", &m);
            schedule_lpt(&tasks, m);
            break;
        case 'S':
            scanf(" %d", &m);
            schedule_spt(&tasks, m);
            break;
        case 'M':
            scanf(" %d", &m);
            schedule_mcnaughton(&tasks, m);
            break;
        case 'A':
            scanf(" %d", &m);
            schedule_optimal(&tasks, m);
            break;
        }
    }

    free(tasks.data);
    return 0;
}