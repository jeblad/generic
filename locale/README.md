# Translation Guidelines

## Dash conventions

The English source strings use **em dash** (—) without spaces. This follows the American English
typographic standard (AP/Chicago style) and was chosen deliberately: em dash is visually distinct
from en dash (used in numeric ranges, e.g. 1–10) and from hyphen, so the role of the punctuation
is unambiguous even in a monospace terminal. Translations should use the typographic convention of
the target language:

| Language | Dash | Example |
| --- | --- | --- |
| English (source) | em dash — | `Use --force — data will be lost.` |
| Norwegian (nb, nn) | en dash – with spaces | `Bruk --force – data går tapt.` |
| Danish (da) | en dash – with spaces | `Brug --force – data går tabt.` |
| Swedish (sv) | en dash – with spaces | `Använd --force – data raderas.` |
| German (de) | en dash – with spaces | `--force verwenden – Daten gehen verloren.` |
| French (fr) | em dash — with spaces | `Utiliser --force — les données seront perdues.` |
| Dutch (nl) | en dash – with spaces | `Gebruik --force – gegevens gaan verloren.` |

## Quote conventions

Quotation marks are used for **explicit arguments** — values named inside the sentence (agent
identifiers, filenames, paths) that appear before or around a colon. They are **not** used for
**implicit arguments** — values introduced by a colon at the end of the message.

```text
"Agent '{}' is running."          ← explicit: agent name inside the sentence → quote it
"Failed to open: {}"              ← implicit: colon introduces the value → no quotes
"Failed to read from '{}': {}"    ← first {} is explicit, second is implicit
```

The English source strings use **ASCII single quotes** (`'...'`). This was chosen deliberately:
ASCII single quotes are safe for all C++ compilers and gettext tools without encoding concerns,
they do not conflict with the C++ string delimiter (double quote `"`), and they are clearly
visible in monospace terminals. Translations should use the typographic quotation marks of the
target language:

| Language | Quotes | Example |
| --- | --- | --- |
| English (source) | `'...'` ASCII single | `Agent 'r2d2' is running.` |
| Norwegian (nb, nn) | «…» | `Agenten «r2d2» kjører.` |
| Danish (da) | »…« | `Agenten »r2d2« kører.` |
| Swedish (sv) | "…" | `Agenten "r2d2" kör.` |
| German (de) | „…" | `Agent „r2d2" läuft.` |
| French (fr) | « … » (with spaces) | `L'agent « r2d2 » est en cours.` |
| Dutch (nl) | '…' | `De agent 'r2d2' draait.` |

Do **not** use quotation marks around commands or code — use backticks instead (see below).

## Code and commands

Use **backticks** for commands, subcommands, flags, and any code in both source strings and
translations. This applies regardless of the target language's normal quotation style, and
regardless of whether the name is a runtime value (`{}` placeholder) or a static literal
embedded directly in the string.

```text
`hera rebuild`      — correct
'hera rebuild'      — incorrect
»hera rebuild«      — incorrect
« hera rebuild »    — incorrect
```

This includes subcommand names written as static text:

```text
"Agent file not found for `about` request."    ← correct
"Agent file not found for 'about' request."    ← incorrect
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
