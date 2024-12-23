# tt -- simple script to create aliases for ag matches

Essentially a rewrite of tag which hasn't been updated for ages.

## Dependencies
- [ag](https://github.com/ggreer/the_silver_searcher)
- [bash](https://www.gnu.org/software/bash/manual/bash.html) (rewrite to zsh will happen)

Other: creates a temporary file is /tmp/ttag_aliases

## installation (in zsh)

Add function to `.zshrc`:

```zsh
tt() {
  command /path/to/tt.sh "$@"; source /tmp/ttag_aliases 
}
```
