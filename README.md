# tt -- simple script to create aliases for ag matches

Essentially a rewrite of [tag](https://github.com/aykamko/tag) which hasn't been updated for ages.
(Not adding any functionality -- most likely doing a subpar job, but decreasing dependencies.)

## Dependencies
- [ag](https://github.com/ggreer/the_silver_searcher)
- [bash](https://www.gnu.org/software/bash/manual/bash.html) or [zsh](https://www.zsh.org/) or C/POSIX

## Notes

- Creates a temporary file is /tmp/ttag_aliases
- Considerably slower than tag because it is implemented in shell script (but has fewer dependencies)

## installation (in zsh)

Add function to `.zshrc`:

```zsh
tt() {
  command /path/to/tt.sh "$@"; source /tmp/ttag_aliases 
}
```
