# Translation Guidelines

## Dash conventions

The English source strings use **em dash** (—) without spaces, following American English convention.
Translations should use the typographic convention of the target language:

| Language | Dash | Example |
| --- | --- | --- |
| English (source) | em dash — | `Use --force — data will be lost.` |
| Norwegian (nb, nn) | en dash – with spaces | `Bruk --force – data går tapt.` |
| Danish (da) | en dash – with spaces | `Brug --force – data går tabt.` |
| Swedish (sv) | en dash – with spaces | `Använd --force – data raderas.` |
| German (de) | en dash – with spaces | `--force verwenden – Daten gehen verloren.` |
| French (fr) | em dash — with spaces | `Utiliser --force — les données seront perdues.` |
| Dutch (nl) | en dash – with spaces | `Gebruik --force – gegevens gaan verloren.` |

## Code and commands

Use **backticks** for commands, subcommands, flags, and any code in both source strings and
translations. This applies regardless of the target language's normal quotation style.

```text
`hera rebuild`      — correct
'hera rebuild'      — incorrect
»hera rebuild«      — incorrect
« hera rebuild »    — incorrect
```

## Hint messages

Hints are supplementary messages shown in the terminal. They follow the pattern:

```text
[situation] — [action]
```

Do **not** add any prefix to the translation — no "hint:", "tips:", "Hinweis:", "conseil:", or
equivalent. The role of the message is clear from context. The source string has no prefix either.

## Plural forms

Strings that count items use `msgid_plural` with **indexed placeholders**:

- `{0}` — the count (number of items)
- `{1}` — the secondary argument (e.g., a path)

The singular form writes out "one" (or the language equivalent) and omits `{0}`. The formatter
silently ignores unused positional arguments, so both singular and plural receive the same argument
list.

```po
msgid "Exported one agent file to {1}."
msgid_plural "Exported {0} agent files to {1}."
msgstr[0] "Eksporterte én agentfil til {1}."
msgstr[1] "Eksporterte {0} agentfiler til {1}."
```

## Format strings

Non-plural strings use non-indexed `{}` placeholders by default. Do not change the **number**
of placeholders. If the target language requires a different word order, switch to indexed
placeholders (`{0}`, `{1}`, …) in the translation — the formatter accepts both forms.

Plural strings use indexed `{0}`, `{1}` placeholders precisely to allow reordering. Translators
should reorder indexed placeholders freely to produce idiomatic phrasing.
