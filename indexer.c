#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
struct link
{
    char url[2000];
    int count;
    struct link *next;
};
struct word
{
    char w[50];
    struct link *head;
};
void getresult(char *html,char *result,int start,int end)
{
    int i,j=0;
    int tagstart=0;
    for(i=start;i<=end;i++)
    {
        if(html[i]=='<')
        tagstart=1;
        else if(html[i]=='>')
        {
            tagstart=0;
            continue;
        }
        if(tagstart==1)
        continue;
        else
        {
            result[j]=html[i];
            j++;
        }
    }
    result[j]='\0';
    printf("%s\n",result);
}
int findtag(char *html, char *result, int pos,char *c)
{
    int titlestart = -1;
    int titleend = -1;
    int i, j;
    int temp;
    int check = 0;
    printf("inside findtag-----tag to search is %s\n",c);
    int k = 0;
    for (i = pos; html[i] != EOF; i++)
    {
        if (html[i] == '<' && html[i + 1] == c[0])
        {
            k = 0;
            check = 0;
            for (j = i + 1; html[j] != '>' && html[j] != EOF && k < strlen(c); j++)
            {
                if (html[j] != c[k])
                {
                    check = 1;
                    break;
                }
                k++;
            }
            if(k==strlen(c))
            {
                for(;html[j] != '>' && html[j] != EOF;j++)
                {   }
            }
            i = j - 1;
            if (check == 0)
            {
                titlestart = j + 1;
            }
        }
        if (html[i] == '<' && html[i + 1] == '/' && html[i + 2] == c[0])
        {
            k = 0;
            check = 2;
            temp=i;
            for (j = i + 2; html[j] != '>' && html[j] != EOF && k < 5; j++)
            {
                if (html[j] != c[k])
                {
                    check = 3;
                    break;
                }
                k++;
            }
            i = j - 1;
            if (check == 2)
            {
                titleend = temp - 1;
                if (titleend > titlestart)
                {
                    getresult(html,result,titlestart,titleend);
                    return i;
                }
            }
        }
    }
    return -1;
}
int main()
{
    FILE *f;
    char dir[] = "/home/nikhil/Desktop/APC/SearchEngine/";
    int filecount = 1;
    char filechar[5];
    char open[250];
    char url[2250];
    char *html;
    int a=0;
    while (1)
    {
        sprintf(filechar, "%d", filecount);
        strcpy(open, dir);
        strcat(open, filechar);
        strcat(open, ".txt");
        // calculating size of file
        struct stat st;  //variable which will count length of file.
        stat(open, &st); // temp.txt is the file where wget fetch the html
        int size;
        size = st.st_size + 100;
        //creating htmlfuffer
        html = (char *)malloc(sizeof(char) * size);
        struct word *head=NULL;        
        //open file to read
        f = fopen(open, "r");
        if (f == NULL)
        {
            printf("File could not be opened\ncount is %d\n",a);
            return 1;
        }
        a++;
        int i = 0;
        fgets(url,2250,f);
        url[strlen(url)-1]='\0';
        html[i] = fgetc(f);
        while (html[i] != EOF)
        {
            i++;
            html[i] = fgetc(f);
        }
        fclose(f);
        int position = 0;
        char title[]="title";
        char tags[][7]={"title","h1","h2","h3","h4","h5"};
        char *result = (char *)malloc(sizeof(char) * 1500);
        for(int r=0;r<6;r++)
        {
            position=0;
            printf("r - %d and %s\n",r,tags[r]);
            while (position != -1)
            {
                position = findtag(html, result, position,tags[r]);
            }
        }
        filecount++;
        free(html);
    }
}