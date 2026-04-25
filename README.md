<div align="center">
<h2>FTIndex</h2>
A multi-threaded full-text indexer written in C.
</div>

## 💡 Overview
FTIndex builds a full-text search index from multiple input text files. 

It creates a shared hash table storing pairs:

$\ \langle word, file\_name \rangle$

Each word is mapped to all files in which it appears, enabling fast lookup of words across multiple documents.

The program uses **one thread per input file** and a **thread-safe hash table with separate chaining**.


## 📖 Word definition
A word is defined as a contiguous sequence of alphanumeric characters extracted from the input text.

**Constraints:**
- Maximum word length: **1024 characters**
- Words are **case-sensitive**
- Non-alphanumeric characters (e.g. punctuation) are treated as delimiters

## ⚙️ How it works
- Each input file is processed in a separate thread
- Words are extracted and inserted into a shared hash table
- Each hash bucket is protected with a mutex (fine-grained locking)
- After processing, an inverted index is printed

## 📤 Output format
The output is printed in the following format:
```
word: file1 file2 file3 ...
```

**Example:**
```
adipiscing: lipsum1.txt lipsum2.txt
consectetuer: lipsum1.txt lipsum2.txt
facilisis: lipsum1.txt
leo: lipsum1.txt
```

## 🧰 Dependencies
- GCC (C compiler with C99 support)
- Make
- POSIX-compliant system (Linux recommended, required for pthreads)

## 🛠️ Build
```bash
git clone https://github.com/LisyFOX/ftindex.git
cd ftindex
make
```

## ▶️ Usage
```bash
./ftindex <hash_table_size> <path_to_file1> <path_to_file2> ...
```
**Example:**
```bash
./ftindex 10 lipsum1.txt lipsum2.txt
```