#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

struct list
{
  struct list *prev;
  struct list *next;
  char url[2000];
  int depth;
  int key;
  int visitedflag;
};

int checkurl(char *url)
{
  //Function to check if the entered url by user is valid or not
  char wget[2050] = {"wget --spider "};
  strcat(wget, url);
  if (!system(wget))
  {
    //printf("Valid URL\n");
    return 1;
  }
  else
    //printf("Invalid URL\n");
    return 0;
}

void getpage(char *url)
{
  char urlbuffer[2100] = {0};
  strcat(urlbuffer, "wget -O ");
  strcat(urlbuffer, "/home/nikhil/Desktop/APC/SearchEngine/temp.txt ");
  strcat(urlbuffer, url);
  printf("the wget command becomes %s\n", urlbuffer);
  // strcat(urlbuffer, " --proxy-user=user1 --proxy-password=user1");
  system(urlbuffer);
}

void testDir(char *dir)
{
  //Function to check if the directory entered by user is correct or not
  struct stat statbuf;
  if (stat(dir, &statbuf) == -1)
  {
    fprintf(stderr, "-----------------\n");
    fprintf(stderr, "Invalid directory\n");
    fprintf(stderr, "-----------------\n");
    exit(1);
  }

  //Both check if there's a directory and if it's writable
  if (!S_ISDIR(statbuf.st_mode))
  {
    fprintf(stderr, "-----------------------------------------------------\n");
    fprintf(stderr, "Invalid directory entry. Your input isn't a directory\n");
    fprintf(stderr, "-----------------------------------------------------\n");
    exit(1);
  }

  if ((statbuf.st_mode & S_IWUSR) != S_IWUSR)
  {
    fprintf(stderr, "------------------------------------------\n");
    fprintf(stderr, "Invalid directory entry. It isn't writable\n");
    fprintf(stderr, "------------------------------------------\n");
    exit(1);
  }
}

void removeWhiteSpace(char *html)
{
  int i;
  char *buffer = malloc(strlen(html) + 1), *p = malloc(sizeof(char) + 1);
  memset(buffer, 0, strlen(html) + 1);
  for (i = 0; html[i]; i++)
  {
    if (html[i] > 32)
    {
      sprintf(p, "%c", html[i]);
      strcat(buffer, p);
    }
  }
  strcpy(html, buffer);
  free(buffer);
  free(p);
}

int GetNextURL(char *html, char *urlofthispage, char *result, int pos)
{
  char c;
  int len, i, j;
  char *p1; //!< pointer pointed to the start of a new-founded URL.
  char *p2; //!< pointer pointed to the end of a new-founded URL.

  // NEW
  // Clean up \n chars
  if (pos == 0)
  {
    removeWhiteSpace(html);
  }
  // /NEW

  // Find the <a> <A> HTML tag.
  while (0 != (c = html[pos]))
  {
    if ((c == '<') &&
        ((html[pos + 1] == 'a') || (html[pos + 1] == 'A')))
    {
      break;
    }
    pos++;
  }
  //! Find the URL it the HTML tag. They usually look like <a href="www.abc.com">
  //! We try to find the quote mark in order to find the URL inside the quote mark.
  if (c)
  {
    // check for equals first... some HTML tags don't have quotes...or use single quotes instead
    p1 = strchr(&(html[pos + 1]), '=');

    if ((!p1) || (*(p1 - 1) == 'e') || ((p1 - html - pos) > 10))
    {
      // keep going...
      return GetNextURL(html, urlofthispage, result, pos + 1);
    }
    if (*(p1 + 1) == '\"' || *(p1 + 1) == '\'')
      p1++;

    p1++;

    p2 = strpbrk(p1, "\'\">");
    if (!p2)
    {
      // keep going...
      return GetNextURL(html, urlofthispage, result, pos + 1);
    }
    if (*p1 == '#')
    { // Why bother returning anything here....recursively keep going...

      return GetNextURL(html, urlofthispage, result, pos + 1);
    }
    if (!strncmp(p1, "mailto:", 7))
      return GetNextURL(html, urlofthispage, result, pos + 1);
    if (!strncmp(p1, "http", 4) || !strncmp(p1, "HTTP", 4))
    {
      //! Nice! The URL we found is in absolute path.
      strncpy(result, p1, (p2 - p1));
      return (int)(p2 - html + 1);
    }
    else
    {
      //! We find a URL. HTML is a terrible standard. So there are many ways to present a URL.
      if (p1[0] == '.')
      {
        //! Some URLs are like <a href="../../../a.txt"> I cannot handle this.
        // again...probably good to recursively keep going..
        // NEW

        return GetNextURL(html, urlofthispage, result, pos + 1);
        // /NEW
      }
      if (p1[0] == '/')
      {
        //! this means the URL is the absolute path
        for (i = 7; i < strlen(urlofthispage); i++)
          if (urlofthispage[i] == '/')
            break;
        strcpy(result, urlofthispage);
        result[i] = 0;
        strncat(result, p1, (p2 - p1));
        return (int)(p2 - html + 1);
      }
      else
      {
        //! the URL is a absolute path.
        len = strlen(urlofthispage);
        for (i = (len - 1); i >= 0; i--)
          if (urlofthispage[i] == '/')
            break;
        for (j = (len - 1); j >= 0; j--)
          if (urlofthispage[j] == '.')
            break;
        if (i == (len - 1))
        {
          //! urlofthis page is like http://www.abc.com/
          strcpy(result, urlofthispage);
          result[i + 1] = 0;
          strncat(result, p1, p2 - p1);
          return (int)(p2 - html + 1);
        }
        if ((i <= 6) || (i > j))
        {
          //! urlofthis page is like http://www.abc.com/~xyz
          //! or http://www.abc.com
          strcpy(result, urlofthispage);
          result[len] = '/';
          strncat(result, p1, p2 - p1);
          return (int)(p2 - html + 1);
        }
        strcpy(result, urlofthispage);
        result[i + 1] = 0;
        strncat(result, p1, p2 - p1);
        return (int)(p2 - html + 1);
      }
    }
  }
  return -1;
}

void NormalizeWord(char *word)
{
  int i = 0;
  while (word[i])
  {
    // NEW
    if (word[i] < 91 && word[i] > 64) // Bounded below so this funct. can run on all urls
      // /NEW
      word[i] += 32;
    i++;
  }
}

int NormalizeURL(char *URL)
{
  int len = strlen(URL);
  if (len <= 1)
    return 0;
  //! Normalize all URLs.
  if (URL[len - 1] == '/')
  {
    URL[len - 1] = 0;
    len--;
  }
  int i, j;
  len = strlen(URL);
  //! Safe check.
  if (len < 2)
    return 0;
  //! Locate the URL's suffix.
  for (i = len - 1; i >= 0; i--)
    if (URL[i] == '.')
      break;
  for (j = len - 1; j >= 0; j--)
    if (URL[j] == '/')
      break;
  //! We ignore other file types.
  //! So if a URL link is to a file that are not in the file type of the following
  //! one of four, then we will discard this URL, and it will not be in the URL list.
  if ((j >= 7) && (i > j) && ((i + 2) < len))
  {
    if ((!strncmp((URL + i), ".htm", 4)) || (!strncmp((URL + i), ".HTM", 4)) || (!strncmp((URL + i), ".php", 4)) || (!strncmp((URL + i), ".jsp", 4)))
    {
      len = len; // do nothing.
    }
    else
    {
      return 0; // bad type
    }
  }
  return 1;
}
int calculatekey(char *url)
{
  int sum = 0, i;
    for (i = 0; url[i] != '\0'; i++)
    {
      sum += url[i];
    }
    while (sum > 9)
    {
      sum = sum / 10;
    }
    return sum;
}
int createfile(char **htmlbuffer, int *filecount,char *url)
{
  //printf("\n----File count received in function is %d-----\n",*filecount);
  int size;
  //static int filecount=0;
  char filechar[10];
  sprintf(filechar, "%d", *filecount);
  strcat(filechar, ".txt");
  char final[200] = "touch /home/nikhil/Desktop/APC/SearchEngine/";
  strcat(final, filechar);
  printf("\n----Command run to create file is --- %s\n", final);
  system(final);

  // calculating size of file
  struct stat st;                                              //variable which will count length of file.
  stat("/home/nikhil/Desktop/APC/SearchEngine/temp.txt", &st); // temp.txt is the file where wget fetch the html
  size = st.st_size + 10;

  //copying data
  char *c = (char *)malloc(sizeof(char) * size);
  FILE *fptr1, *fptr2;
  char temp[] = "/home/nikhil/Desktop/APC/SearchEngine/temp.txt";
  fptr1 = fopen(temp, "r");
  if (fptr1 == NULL)
  {
    printf("Cannot open file %s \n", temp);
    exit(0);
  }
  strcpy(final, "/home/nikhil/Desktop/APC/SearchEngine/");
  strcat(final, filechar);
  fptr2 = fopen(final, "w");
  if (fptr2 == NULL)
  {
    printf("Cannot open file %s \n", final);
    exit(0);
  }
  char x;
  int i = 0;
  c[i] = fgetc(fptr1);
  while (c[i] != EOF)
  {
    fputc(c[i], fptr2);
    i++;
    c[i] = fgetc(fptr1);
  }
  *htmlbuffer = c;
  *filecount = *filecount + 1;
  printf("\nFile count incremented by receiving in function is %d\n", *filecount);
  return size;
}

struct list *insertlist(struct list **head, char *url, int currentdepth)
{
  if ((*head) == NULL)
  {
    (*head) = (struct list *)malloc(sizeof(struct list));
    (*head)->next = NULL;
    (*head)->prev = NULL;
    //(*head)->url=url;
    strcpy((*head)->url, url);
    (*head)->visitedflag = 0;
    (*head)->depth = currentdepth + 1;
    //calculating and inserting key
    (*head)->key = calculatekey(url);
    //printf("%s\n",(*head)->url);
    //printf("\n**********Inserting link in linked list***************\n");
    return (*head);
  }
  else
  {
    struct list *temp = (*head);
    while (1)
    {
      if (!strcmp(temp->url, url))
      {
        //printf("\n***********Inserting link in linked list----found duplicate\n");
        return NULL;
      }
      if (temp->next == NULL)
        break;
      else
      {
        temp = temp->next;
      }
    }
    struct list *x = temp;
    temp->next = (struct list *)malloc(sizeof(struct list));
    temp = temp->next;
    temp->next = NULL;
    temp->prev = x;
    strcpy(temp->url, url);
    temp->visitedflag = 0;
    temp->depth = currentdepth + 1;
    //calculating and inserting key
    temp->key = calculatekey(url);
    return temp;
  }
}
void printresult(char *x)
{
  for (int i = 0; x[i] != '\0'; i++)
  {
    if (!(x[i] >= 32 && x[i] <= 126))
    {
      printf("-u-o-%u", x[i]);
    }
  }
  printf("\n");
}
int getnexturlfromlist(struct list *head, int currdepth,char *url)
{
  printf("\n\n\n||||||||||||||Inside getnext url||||||||\n\n\n");
  struct list *temp = head;
  while (temp != NULL)
  {
    if (temp->visitedflag == 0 && temp->depth == currdepth)
    {
      temp->visitedflag = 1;
      //char *res = (char *)malloc(sizeof(char) * 2000);
      strcpy(url, temp->url);
      //return res;
      return 1;
    }
    temp = temp->next;
  }
  return 0;
}
int main()
{
  char input;
  int i, maxdepth=5, filecount = 1;
  int currentdepth;
  char *url = (char *)malloc(sizeof(char) * 200);
  char dir[200];
  //char result[2000];
  char *result = (char *)malloc(sizeof(char) * 2000);
  //result=NULL;
  struct list *head = NULL;
  struct list *tail = NULL;
  struct list *temp;
  char *store[100];
  for (i = 0; i < 100; i++)
  {
    store[i] = (char *)malloc(sizeof(char) * 2000);
  }
  printf("Do you want to load list ?? (y/n)\n");
  scanf("%c ", &input);
  printf("%c\n",input);
  if (input == 'y' || input == 'Y')
  {
    //Fetch the link list from the file.
    //Insert all the data in a data structure.
  }
  else
  {
    printf("Enter seed url\n");
    //scanf("%s",url);
    strcpy(url, "www.chitkara.edu.in/");
    printf("Enter depth\n");
    //scanf("%d",&depth);
    maxdepth = 5;
    printf("Enter file directory\n");
    //scanf("%s",dir);
    strcpy(dir, "/home/nikhil/Desktop/APC/SearchEngine/");
    if (maxdepth < 1 || maxdepth > 5)
    {
      printf("Depth value incorrect.");
      return 0;
    }
    testDir(dir);
    if (!checkurl(url))
      return 1;
    temp = insertlist(&head, url, 0);
    if (temp != NULL)
      tail = temp;
    tail->visitedflag = 1;
    currentdepth=1;
  }

  //************************Next Phase***************************//
  //From here both the cases of reading from a link list or a new one combines
  int sizeofstorage=20;
  while (currentdepth<maxdepth)
  {
    printf("\n\n\n############ get page########\n\n\n");
    getpage(url);
    int filesize = 0;
    char *htmlbuffer;
    filesize = createfile(&htmlbuffer, &filecount,url);
    printf("\n\nFile created-------File count is %d\n\n", filecount);
    int pos;
    pos=0;
    i = 0;
    int count,linkcount;
    count=0;
    linkcount=0;
    while (pos < filesize)
    {
      //result=" ";
      if (linkcount == sizeofstorage)
      {
        printf("---Count is %d---\n", count);
        break;
      }
      pos = GetNextURL(htmlbuffer, url, result, pos);
      if (pos == -1)
      {
        printf("No link found\n");
        break;
      }
      else
      {
        count++;
        printf("Resultant res url is %s\n---- ", result);
        printresult(result);
        if (checkurl(result))
        {
          printf("\n\nstoring result %s\n\n", result);
          strcpy(store[i], result);
          printf("\n\nstored storage value is %s\n\n", store[i]);
          printf("\n????url added???\n");
          linkcount++;
          i++;
          //result=NULL;
        }
        pos++;
      }
    }
    //inserting links in linked list
    for (int j = 0; j < linkcount; j++)
    {
      //printf("Url to insert is ----  %s",store[j]);
      temp = insertlist(&head, store[j], currentdepth);
      if (temp != NULL)
        tail = temp;
    }

    temp = head;
    while (temp != NULL)
    {
      printf("\n\nurl is %s\n", temp->url);
      printf("Depth is %d\n",temp->depth);
      temp = temp->next;
    }
    printf("Do you want to quit? (y/n)\n");
    scanf("%c",&input);
    if (input == 'y' || input == 'Y')
    {
      //save the link list to the file and exit
      break;
    }

    //free pointers !!!
    free(htmlbuffer);
    while(currentdepth<maxdepth)
    {
      if(getnexturlfromlist(head,currentdepth,url))
      {
        printf("Url found----terminating loop\n");
        break;
      }
      else
      {
        currentdepth++;
        sizeofstorage-=5;
      }
    }
    printf("\n\n\n||||||||||||||outside getnext url||||||||\n\n\n");
    printf(" Current depth is %d\n\n",currentdepth);
  }
  free(result);
  temp = head;
  while (temp != NULL)
  {
    free(temp->prev);
    temp = temp->next;
  }
  free(tail);
  for (int i = 0; i < 100; i++)
  {
    free(store[i]);
  }
  free(url);
  return 0;
}
















//temp=head;
/*
{
  //creating hash table
  struct list *searchhash[10];
    for(i=0;i<10;i++)
    {
      searchhash[i]=NULL;
    }
  //when passing the url to insert in the link list also pass thiss hash table
  int sum=0,i;
  for(i=0;string[i]!='\0';i++)
  {
    sum+=string[i];
  }
  while(sum>9)
  {
    sum=sum/10;
  }
  //this sum becomes the key now at which the url node is to be checked and inserted
  struct list *temp=searchhash[key];
  while(temp->key==key)
  {
    int x=strcmp(temp->url,string);
    if(x==0)
    {
      printf("url already exists\n");
      return;
    }
    temp=temp->next;
  }
  struct list *node=(struct list*)malloc(sizeof(struct list)*1);
  node->next=temp;
  node->prev=temp->prev;
  temp->prev=node;
  node->prev->next=node;
  node->key=key;
  node->url=string;
  node->depth=currentdepth;
}
*/