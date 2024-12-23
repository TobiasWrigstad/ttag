#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <assert.h>

#define BLUE    "\033[34m"
#define RESET   "\033[0m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define GREEN   "\033[32m"
#define HIGHLIGHT "\033[30;43m"

const char *alias_file = "/tmp/ttag_aliases";

// Function to add ANSI escape sequences to the input string
void highlight(const char *input, const char *pattern, char *output, size_t maxlen) {
  regex_t regex;
  regmatch_t matches[2];  // 2 groups: whole match and group 1

  // Compile the regular expression
  if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE))
    {
      perror("Could not compile regex");
      abort();
    }

  const char *start = input;
  char *out_ptr = output;
  size_t bytes_remaining = maxlen - strlen(start) - 1; // Extra -1 for \0

  // Loop through the input string and find matches
  while (regexec(&regex, start, 2, matches, 0) == 0)
    {
      if (bytes_remaining < 15) break;
      bytes_remaining -= 15; // Each match adds this many characters

      // Copy text before the match (unmodified)
      int prefix_len = matches[0].rm_so;
      strncpy(out_ptr, start, prefix_len);
      out_ptr += prefix_len;

      // Add the ANSI escape sequence before the match
      strcpy(out_ptr, HIGHLIGHT);
      out_ptr += strlen(HIGHLIGHT);

      // Copy the matched portion (group 1)
      int match_len = matches[0].rm_eo - matches[0].rm_so;
      strncpy(out_ptr, start + matches[0].rm_so, match_len);
      out_ptr += match_len;

      // Add the reset escape sequence after the match
      strcpy(out_ptr, RESET);
      out_ptr += strlen(RESET);

      // Move start position after the match
      start += matches[0].rm_eo;
    }

  // Copy the remaining part of the string (after last match)
  strncpy(out_ptr, start, bytes_remaining);

  // Free regex memory
  regfree(&regex);
}

void process_line(char *line, char *search_pattern, char *editor, FILE *aliases)
{
  static char *file_name = NULL;
  static int match_idx = 0;

  assert(file_name || match_idx == 0);

  regex_t regex;
  const char *pattern = "^([0-9]+):([0-9]+):(.+)$";  // Regular expression pattern
  regmatch_t matches[4];  // Array to hold match results (3 groups + full match)

  int ret = regcomp(&regex, pattern, REG_EXTENDED);
  if (ret)
    {
      fprintf(stderr, "Could not compile regex\n");
      abort();
    }

  // Execute the regular expression
  ret = regexec(&regex, line, 4, matches, 0);
  if (!ret)
    {
      match_idx += 1;

      int lineno_len = matches[1].rm_eo - matches[1].rm_so;
      char lineno_str[lineno_len + 1];
      snprintf(lineno_str, lineno_len + 1, "%.*s", lineno_len, line + matches[1].rm_so);
      int lineno_int = atoi(lineno_str);

      int colno_len = matches[2].rm_eo - matches[2].rm_so;
      char colno_str[colno_len + 2];
      snprintf(colno_str, colno_len + 2, "%.*s", colno_len, line + matches[2].rm_so);
      int colno_int = atoi(colno_str);

      int match_len = matches[3].rm_eo - matches[3].rm_so;
      char match_str[match_len + 3];
      snprintf(match_str, match_len + 3, "%.*s", match_len, line + matches[3].rm_so);

      char match_str_highlighted[512] = {0};
      highlight(match_str, search_pattern, match_str_highlighted, 512);

      printf("%s[%s%d%s] %s%d: %s\n", BLUE, MAGENTA, match_idx, BLUE, RESET, lineno_int, match_str_highlighted);
      fprintf(aliases, "alias e%d='%s %s:%d:%d'\n", match_idx, editor, file_name, lineno_int, colno_int);
    }
  else if (ret == REG_NOMATCH)
    {
      // This is (we assume given how ag works) a filename.ext -- so set that
      // as the current file name
      char *nl = strchr(line, '\n');
      if (nl) *nl = '\0';
      if (file_name) free(file_name);
      file_name = strdup(line);
      printf("%s%s%s\n", GREEN, file_name, RESET);
    }
  else
    {
      char errbuf[100];
      regerror(ret, &regex, errbuf, sizeof(errbuf));
      printf("Regex match failed: %s\n", errbuf);
    }

  // Free the memory allocated to the regex structure
  regfree(&regex);
}

const char *AG = "/usr/bin/ag --column --group";

int main(int argc, char** argv)
{
  if (argc <= 1)
    {
      puts("Usage: tt <args> (see man ag for argument syntax)");
    }
  else
    {
      char *editor = getenv("EDITOR");
      if (editor == NULL)
        {
          puts("Cannot continue: $EDITOR local environment variable not set!");
          return -1;
        }

      size_t command_len = strlen(AG);
      size_t command_buf_siz = command_len + 128;
      char *command = calloc(command_buf_siz, sizeof(char));
      strcat(command, AG);
      for (int i = 1; i < argc; ++i)
        {
          size_t inc_bytes = strlen(argv[i]) + 1; // +1 for space
          if (command_len + inc_bytes > command_buf_siz)
            {
              command_len += inc_bytes;
              command_buf_siz += inc_bytes;
              command = realloc(command, command_buf_siz);
            }

          strcat(command, " ");
          strcat(command, argv[i]);
        }

      FILE *search_results = popen(command, "r");
      FILE *aliases = fopen("/tmp/ttag_aliases", "w");

      free(command);

      for (;;)
        {
          char *buf = NULL;
          size_t len = 0;

          if (getline(&buf, &len, search_results) >= 0)
            {
              // Prune \n character
              char *nl = strchr(buf, '\n');
              if (nl) *nl = '\0';

              // If line was empty then buf == nl
              if (buf < nl)
                {
                  process_line(buf, argv[argc-1], editor, aliases);
                }
              else
                {
                  puts(""); // Blank line
                }

              free(buf);
            }
          else
            {
              free(buf);
              break; // done processing
            }
        }

      fclose(aliases);
      pclose(search_results);
    }

  return 0;
}
