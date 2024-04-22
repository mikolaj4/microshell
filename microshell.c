#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <regex.h>
#include <time.h>
#include <fcntl.h>
 
char *alias_names[10];
char *alias_contents[10];
char *alias_name;
char *alias_content;
int alias_counter_main;
char historia[100][50];

int ReadInput(char **args)	//czyta wejście i dzieli umieszcza każde słowo oddzielone spacją pod osobnm indeksem tablicy *args[]
{
	char all[256];
	int ktore_slowo=0, indeks_literki=0;
	char temp[20][50];
	fgets(all, 256, stdin);
	all[strlen(all)-1] = '\0';

	if(strlen(all)==0)
		return -1;

	for(int i=0; i<=(strlen(all)); i++)
	{
		if(all[i]==' ' || all[i] == '\0')
		{
			temp[ktore_slowo][indeks_literki] = '\0';
			indeks_literki=0;
			ktore_slowo++;
		}
		else
		{
			temp[ktore_slowo][indeks_literki] = all[i];
			indeks_literki++;
		}
	}
	for(int i=0; i<ktore_slowo; i++)
	{
		args[i] = temp[i];
	}
	return ktore_slowo;

}

void Help()
{
	printf("-----Projekt Microshell SOP 2022-----\n"
	"Autor: Mikołaj Gawor\n\n"
	"-----Oferowane funkcjonalności-----\n"
	"	-Autorska funkcja alias z wyłapywaniem błędów w składni. Wpisz alias help po wiecej info.\n"
	"	-Autorska komenda his. Wpisz his aby zobaczyć historię wykonanych poleceń.\n"
	"	-Autorski program mcp działający analogicznie jak cp znane z bash.\n"
	"	-Po zakończeniu exitem wyświetla się informacje przez jaki czas program był aktywny.\n"
	"\n"
	);

}


void ExternalProgram(char **args, int args_number) //uruchamia program zewnętrzny
{
	pid_t pid = fork();

	if(pid<0)
	{
		printf("Failed to execute fork()\n");
		exit(EXIT_FAILURE);
		return;
	}
	else if(pid==0)
	{
		if(args_number==1)
		{
			if((execlp(args[0], args[0], NULL))==-1)
			{
				perror("Nie można zinterpretować polecenia");
				printf("Kod błędu: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}
		else if(args_number>1)
		{
			args[args_number] = NULL;
			if((execvp(args[0], args))==-1)
			{
				perror("Nie można zinterpretować polecenia: ");
				printf("Kod błędu: %d\n", errno );
				exit(EXIT_FAILURE);
			}
		}
	}

	wait(NULL);
	return;
}

void AliasPrintHelp()
{
		printf(
		"alias	<- wyświetla listę aktualnych aliasów\n"
		"alias nazwa='twoj alias'	<- tworzy alias\n"
		"unalias nazwa	<- usuwa alias o podanej nazwie\n\n"
		"nazwa aliasu może zawierać:\n"
		"	-małe litery\n"
		"	-DUŻE LITERY\n"
		"	-cyfry\n"

		);
}

void MyOwnAlias(char **args, int args_number, int is_unalias)
{

	if(args_number==1 && is_unalias==0) //wyświeltam listę istniejących aliasów
	{
		for(int i=0; i!=alias_counter_main; i++)
		{
			if(strcmp("noshow", alias_names[i]))
			{
				printf("alias %s='%s'\n", alias_names[i], alias_contents[i]);
			}
		}
	}
	else if(args_number==2 && is_unalias==1) //usuwam wskazany alias
	{
		for(int i=0; i!=alias_counter_main; i++)
		{
			if(!strcmp(args[1], alias_names[i]))
			{
				alias_names[i] = "noshow";
				alias_contents[i] = "noshow";
			}
		}
	}
	else if(args_number>1 && is_unalias==0)
	{

		alias_name = malloc(32 * sizeof(*alias_name));
		alias_content = malloc(32 * sizeof(*alias_content));

		char wszystko[128]={};
		for(int i=0; i!=args_number; i++)
		{
			strcat(wszystko, args[i]);
			strcat(wszystko, " ");
		}

		regex_t regex;
		int regcomp_return = regcomp(&regex, "^alias .*='.*' $", 0);
		if(regcomp_return != 0)
		{
			printf("Błąd przy kompilacji regex w celu sprawdzenia poprawności podanego aliasu :(\n");
			return;
		}

		int czy_pasuje = regexec(&regex, wszystko, 0, NULL, 0);

		if(czy_pasuje==REG_NOMATCH)
		{
			AliasPrintHelp();
			return;
		}
		else if(czy_pasuje==0)
		{
			//poprawna składnia alias
			int wszystko_counter=6;
			int c=0;
			while(wszystko[wszystko_counter] != '=')
			{
				alias_name[c] = wszystko[wszystko_counter];
				wszystko_counter++;
				c++;
			}

			c=0;
			int wszystko_index = 8 + strlen(alias_name);
			while(wszystko[wszystko_index] != '\'')
			{
				alias_content[c] = wszystko[wszystko_index];
				c++;
				wszystko_index++;
			}


			if(strlen(alias_name)==0 || strlen(alias_content)==0)
			{
				printf("Nazwa aliasu ani jego zawartość nie może być pusta!\n");
				return;
			}

			alias_names[alias_counter_main]= alias_name;
			alias_contents[alias_counter_main] = alias_content;

			for(int i=0; i!=alias_counter_main; i++)
			{
				if(!strcmp(alias_names[i], alias_name))
				{
					printf("Alias o takiej nazwie już istnieje!\n");
					return;
				}
			}


			alias_counter_main++;

		}
		else
		{
			printf("Błąd przy weryfikacji polecenia alias :(");
			return;
		}
	}
	else
	{
		AliasPrintHelp();
	}

}



void MyCd(char **args, int args_number)
{
	if(args_number==1)
	{
		printf("Brak argumentów!\n");
		return;
	}
	else if(args_number==2)
	{
		if(!strcmp(args[1], "~"))
			args[1] = getenv("HOME");

		int chdir_return_value = chdir(args[1]);
		if(chdir_return_value==-1)
		{
			printf("Błędna nazwa katalogu: %s\n", args[1]);
			return;
		}
	}
	else
	{
		printf("Za dużo argumentów: %d\n", args_number-1);
		return;
	}
	return;

}

void His(int his_c) //wyświetlanie historii
{
	for(int i=0; i!=his_c; i++)
	{
		printf("   [%d]   %s\n", i+1, historia[i]);
	}
}

void DisplayTime(int seconds) //wyświetla czas dziłania programu w formacie minuty, sekundy
{
	int minutes = 0;

	while(seconds>59)
	{
		minutes++;
		seconds = seconds - 60;
	}


		printf("\033[0;36m");
		printf("Program był aktywny przez ");
		printf("\033[0m");

		printf("\033[0;35m");
		printf("%d", minutes);
		printf("\033[0m");

		printf("\033[0;36m");
		printf(" minut i ");
		printf("\033[0m");

		printf("\033[0;35m");
		printf("%d", seconds);
		printf("\033[0m");

		printf("\033[0;36m");
		printf(" sekund.\n");
		printf("\033[0m");
}

void MyCp(char **args, int args_number) //mcp działa tak jak cp z basha
{
	if(args_number!=3)
	{
		printf("Polecenie mcp przyjmuje tylko 2 argumenty!\n");
		return;
	}
	else
	{
		char buffer[2048];
		int fd_odczyt = open(args[1], O_RDONLY);

		if(fd_odczyt==-1)
		{
			printf("Nie udało się odczytać pliku źródłowego!\n");
			return;
		}
		else
		{
			int fd_zapis = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);
			int num;
			while((num = read(fd_odczyt, &buffer, 2048))>0)
			{
				write(fd_zapis, &buffer, num);
			}
			printf("Sukces mycp!\n");


			close(fd_odczyt);
			close(fd_zapis);
		}
	}

}




int main()
{
	alias_names[0] = "init";
	int his_c = 0;
	time_t starttime = time(NULL);


	while(1)
	{
		char *args[128] = {};
		char cwd_name[128];

		if(getcwd(cwd_name, sizeof(cwd_name)) == NULL)
		{
			perror("Nie można pobrać nazwy katalogu: ");
			printf("Kod błędu: %d\n", errno);
			return -1;
		}


		printf("[");
		printf("\033[0;31m");
		printf("%s", getlogin());
		printf("\033[0m");
		printf(":");
		printf("\033[0;33m");
		printf("%s", cwd_name);
		printf("\033[0m");
		printf("]\n");
		printf("$ ");

		int args_number = ReadInput(args);

		if(args_number!=-1)
		{

			for(int i=0; i!=args_number; i++)
			{
				strcat(historia[his_c], args[i]);
				strcat(historia[his_c], " ");
			}
			his_c++;


			for(int i=0; i!=alias_counter_main; i++) //jeśli zawartośc aliasu zawiera spacje np 'ls -l /usr' to rozdzielam poszczególne argumenty
			{
				if((!strcmp(args[0], alias_names[i]))&&args[1]==NULL)
				{
					char all[256] = {};
					strcat(all, alias_contents[i]);

					char temp[20][50] = {};
					int ktore_slowo=0, indeks_literki=0;
					for(int i=0; i<=strlen(all); i++)
					{
						if(all[i]==' ' || all[i]=='\0')
						{
							temp[ktore_slowo][indeks_literki] = '\0';
							indeks_literki=0;
							ktore_slowo++;
						}
						else
						{
							temp[ktore_slowo][indeks_literki] = all[i];
							indeks_literki++;
						}
					}

					for(int i=0; i<ktore_slowo; i++)
					{
						args[i] = temp[i];
						args_number++;
					}
					args_number--;
					break;
				}
			}


			if((!strcmp(args[0], "exit"))&&(args_number==1))
			{
				time_t endtime = time(NULL);
				DisplayTime(endtime-starttime);
				break;
			}
			else if(!strcmp(args[0], "cd"))
			{
				MyCd(args, args_number);
			}
			else if((!strcmp(args[0], "help"))&&(args_number==1))
			{
				Help();
			}
			else if(!strcmp(args[0], "alias"))
			{
				MyOwnAlias(args, args_number, 0);
			}
			else if(!strcmp(args[0], "unalias"))
			{
				MyOwnAlias(args, args_number, 1);
			}
			else if((!strcmp(args[0], "his"))&&(args_number==1))
			{
				His(his_c-1);
			}
			else if(!strcmp(args[0], "mcp"))
			{
				MyCp(args, args_number);
			}
			else
			{
				ExternalProgram(args, args_number);
			}


		}

	}

	free(alias_name);
	free(alias_content);
return 0;
}
