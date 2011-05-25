static int get_pragma_line
(struct cpp_reader* reader, unsigned char* bufpos, size_t buflen)
{
  /* return a 0 terminated string */

  const struct cpp_token* tok;
  unsigned char* newpos;
  size_t toklen;

  while (1)
  {
    tok = cpp_get_token(reader);
    if (tok->type == CPP_PRAGMA_EOL || tok->type == CPP_EOF)
    {
      *bufpos = 0;
      return 0;
    }

    toklen = cpp_token_len(tok);
    if (toklen >= buflen) return -1;

    newpos = cpp_spell_token(reader, tok, bufpos, 0);
    buflen -= (newpos - bufpos);
    bufpos = newpos;
  }

  return -1;
}
