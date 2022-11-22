struct job *dequeue(struct jb_que *queue)
{
    if (queue == NULL)
    {
        return NULL;
    }
    if (queue->sz == 0)
    {
        return NULL;
    }
    struct job *job = queue->jobs[queue->fnt];
    queue->fnt = (queue->fnt + 1) % queue->cpcty;
    queue->sz--;
    return job;
}
void *wt_fr_res(void *arg)
{
    struct job *job = (struct job *)arg;
    if (job == NULL)
    {
        return NULL;
    }
    int status;
    waitpid(job->pid, &status, 0);
    job->end_tm = time(NULL);
    if (WIFEXITED(status))
    {
        job->ext_stat = WEXITSTATUS(status);
        if (job->ext_stat == 0)
        {
            job->crnt_stts = R_SUCC;
        }
        else
        {
            job->crnt_stts = FAILED;
        }
    }
    else
    {
        job->crnt_stts = FAILED;
    }
    sv_jb_inf(job);
    removerunningjob(job);
    sem_post(&rnrs);
}
int en_que(struct jb_que *queue, struct job *job)
{
    if (queue == NULL || job == NULL)
    {
        return R_ERR;
    }
    if (queue->sz == queue->cpcty)
    {
        return R_ERR;
    }
    queue->rear = (queue->rear + 1) % queue->cpcty;
    queue->jobs[queue->rear] = job;
    queue->sz++;
    return R_SUCC;
}
struct job *removerunningjob(struct job *job)
{
    if (job == NULL)
    {
        return NULL;
    }
    int i;
    for (i = INI; i < CUR_JS_RNG; i++)
    {
        if (jobsrunning[i]->jid == job->jid)
        {
            break;
}
    }
    if (i == CUR_JS_RNG)
    {
        return NULL;
    }
    struct job *jobtoremove = copyjob(jobsrunning[i]);
    for (; i < CUR_JS_RNG - 1; i++)
    {
        jobsrunning[i] = jobsrunning[i + 1];
    }
    CUR_JS_RNG--;
    return jobtoremove;
}
void *jb_hdlr(void *arg)
{
    while (1)
    {
        sem_wait(&js_que);
        struct job *job = dequeue(queue);
        sem_wait(&rnrs);
        job->crnt_stts = RUNNING;
        job->strt_tme = time(NULL);
        addrngjb(job);
        int pid = fork();
        if (pid == 0)
        {
            char *outputfile = (char *)malloc(sizeof(char) * MX_JS);
            char *errorfile = (char *)malloc(sizeof(char) * MX_JS);
            char *command = job->jb_cmnd;
            sprintf(outputfile, "%d.out", job->jid);
            sprintf(errorfile, "%d.err", job->jid);
            freopen(outputfile, "w", stdout);
            freopen(errorfile, "w", stderr);
            char *args[100];
            char *token;
            int i = INI;
            token = strtok(command, " ");
            while (token != NULL)
            {
                args[i] = token;
                token = strtok(NULL, " ");
                i++;
            }
            args[i] = NULL;

            execvp(args[0], args);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            job->pid = pid;
            job->tid = pthread_self();
            pthread_t tid;
            pthread_create(&tid, NULL, wt_fr_res, (void *)job);
        }
        else
        {
            printf("fork failed\n");
            job->crnt_stts = FAILED;
        }
    }
    return NULL;
}
struct job *copyjob(struct job *job)
{
    struct job *newjob = (struct job *)malloc(sizeof(struct job));
    newjob->jid = job->jid;
    newjob->jb_cmnd = strdup(job->jb_cmnd);
    newjob->crnt_stts = job->crnt_stts;
    newjob->ext_stat = job->ext_stat;
    newjob->pid = job->pid;
    newjob->tid = job->tid;
    newjob->strt_tme = job->strt_tme;
    newjob->end_tm = job->end_tm;
    return newjob;
}

void createnewlogfile()
{
    FILE *fp;
    fp = fopen("log.txt", "w");
    if (fp == NULL)
    {
        printf("Error opening file.\n");
        return;
    }
    fclose(fp);
}
int main(int argc, char *argv[])
{
    MX_CNCRT_JS = atoi(argv[1]);
    set_jobs(argc, argv);
    sem_init(&rnrs, 0, MX_CNCRT_JS);
    sem_init(&js_que, 0, 0);
    
    queue = (struct jb_que *)malloc(sizeof(struct jb_que));
    queue->cpcty = MxJsInQue;
    queue->sz = INI;
    queue->fnt = INI;
    queue->rear = -1;
    queue->jobs = (struct job **)malloc(sizeof(struct job *) * MxJsInQue);

    jobsrunning = (struct job **)malloc(sizeof(struct job *) * MX_CNCRT_JS);
    createnewlogfile();
    pthread_create(&tid, NULL, jb_hdlr, NULL);
    inpt_hdlr();
    dlt_que(queue);
    return R_SUCC;

}































































