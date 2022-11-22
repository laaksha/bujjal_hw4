int sv_jb_inf(struct job *job)
{
    char info[MX_JS];
    char *status, *strt_tme, *end_tm;
    status = getstatus(job->crnt_stts);
    char buffer[80];
    struct tm *tm_info;
    tm_info = localtime(&job->strt_tme);
    strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", tm_info);
    strt_tme = strdup(buffer);
    tm_info = localtime(&job->end_tm);
    strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", tm_info);
    end_tm = strdup(buffer);
    strt_tme[strcspn(strt_tme, "\n")] = INI;
    end_tm[strcspn(end_tm, "\n")] = INI;
    FILE *fp;
    fp = fopen("log.txt", "a");
    if (fp == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }
    sprintf(info, "%d\t%s\t%s\t%s\t%s\n",
            job->jid, job->jb_cmnd,
            strt_tme, end_tm, status);
    fputs(info, fp);
    fclose(fp);
    return R_SUCC;
}
int dlt_que(struct jb_que *queue)
{
    if (queue == NULL)
    {
        return R_ERR;
    }
    int i;
    for (i = INI; i < queue->sz; i++)
    {
        struct job *job = queue->jobs[(queue->fnt + i) % queue->cpcty];
        free(job->jb_cmnd);
        free(job);
    }
    free(queue->jobs);
    free(queue);
    queue = NULL;

}
void *inpt_hdlr()
{
    char *line = NULL, *token = NULL, *command = NULL;
    size_t len = INI;
    ssize_t read;
    int jid = INI;
    while (1)
    {
        printf("scheduler>");
        read = getline(&line, &len, stdin) != -1;
        if (line[read - 1] == '\n')
        {
            line[read - 1] = '\0';
        }
        token = strtok(line, " \t\r\n\0");
        if (EQUAL(token, "submit"))
        {
            command = strtok(NULL, "\r\n\0");
            struct job *newjob = jobinit(jid++, command);
            if (en_que(queue, newjob) == R_ERR)
            {
                printf("waiting queue is full. wait for some time.\n");
            }
            else
            {
                printf("job %d added to the queue\n", newjob->jid);
                sem_post(&js_que);
            }
        }
        else if (EQUAL(token, "submithistory"))
        {
            FILE *fp;
            char ch;
            fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                printf("Error opening file.\n");
                return NULL;
            }
printf("jid\tcommand\tstrt_tme\tend_tm\tstatus\n");
            while ((ch = fgetc(fp)) != EOF)
            {
                printf("%c", ch);
            }
            fclose(fp);
        }
        else if (EQUAL(token, "showjobs"))
        {
            int i;
            printf("jid\tcommand\t\tstatus\n");
            for (i = INI; i < CUR_JS_RNG; i++)
            {
                printf("%d\t%s\t%s\n", jobsrunning[i]->jid, jobsrunning[i]->jb_cmnd, getstatus(jobsrunning[i]->crnt_stts));
            }
            if (queue == NULL)
            {
                return NULL;
            }
            for (i = INI; i < queue->sz; i++)
            {
                struct job *job = queue->jobs[(queue->fnt + i) % queue->cpcty];
                printf("%d\t%s\t\t%s\n", job->jid, job->jb_cmnd, getstatus(job->crnt_stts));
            }
        }
        else if (EQUAL(token, "exit"))
        {
            break;
        }
        else
        {
            printf("invalid command\n");
        }
    }
    free(line);
    return NULL;

}
void set_jobs(int argc, char *argv[])
{
    int temp = INI;
    MX_CNCRT_JS = sysconf(_SC_NPROCESSORS_ONLN);
    if (argc > 1)
    {
        temp = atoi(argv[1]);
        if (temp > MX_CNCRT_JS)
        {
            printf("No.of jobs shouldn't be > than no.of cores\n");
            printf("Set no.of jobs to %d.\n", MX_CNCRT_JS);
        }
        else if (temp < 0)
        {
            printf("Number of jobs cannot be negative.\n");
            printf("Set no. of jobs to %d.\n", MX_CNCRT_JS);
        }
        else
        {
           MX_CNCRT_JS = temp;
        }
    }
}
int addrngjb(struct job *job)
{
    if (job == NULL)
    {
        return R_ERR;
    }
    if (CUR_JS_RNG == MX_CNCRT_JS)
    {
        return R_ERR;
    }
    jobsrunning[CUR_JS_RNG] = job;
    CUR_JS_RNG++;
    return R_SUCC;
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