*Note: This README is a work in progress still in its early stages, currently only the mutate command is mostly complete*

# mutateplaceholder
As part of a larger project for testing the quality of one's unit tests, this CLI app takes possible mutations to apply to a source file and applies a randomly chosen selection of these mutations to that file. 

#### Per parent app:
>YOUR GOAL: Write mutation tests that cause unwanted
  behavior but go unnoticed by your unit tests. Then,
  add/edit unit tests to catch these mutations. Your
  unit tests are considered high-quality when they
  catch a lot of mutations. So, less is more. Fewer
  unit tests that catch more mutations is higher
  quality than a greater quantity of unit tests.

The [commands](#cli-commands) are `mutate`, `highlight`, `score` and `validate`. Mutate does the actual mutating and the other three are tools to help the user work on customizing their input to the app for optimal yield.

* Mutate a source file based upon mutations from a TSV file
* Score a source file on how many mutations are needed per line
* Validate a mutations TSV file to a source file. That is, ensure that each mutation matches at least one source line and warn if multiple mutations can conflict.
* Highlight a mutation file together with a source file into a side-by-side HTML preview page. This shows how many source lines (and which source lines upon click) are matched by each mutation line and indicate which source lines need mutations.


## Process flow
The `parseArgs()` function parses the CLI arguments and sets different members/methods of a `CLIOptions` object which stores the pertinent values resulting from it's own processing of those arguments. The specified command(i.e `mutate`, `highlight`, `score`, `validate`) then takes the values stored in the `CLIOptions` object, further validates them and executes accordingly.

### Mutate command
The mutate command mutates a source file using mutations supplied via a TSV file.  
Each row of the TSV file must contain a pattern cell and at least 1 permutation cell.  
The pattern cell contains what the mutate command looks for to replace in the source file and the permutation cells contain possible replacements with which to replace the pattern with.  
Example, the first cells in each row are the pattern cells and the cells following them are possible permutation cells:  
```
int a = 0;		int a = 10;		int a = 20;  
int sum = a + b;	int sum = a / b;	int sum = a * b;	int sum = a - b;
const char* str="w";	const char* str="u";
```
Single line cells can be quoted or unquoted. Multi-line cells must be quoted.  
Pattern cells may be plain text or regex. Permutation cells for a regex pattern cell may be regex or plain text.

#### After capturing the TSV file's rows, the mutate command will randomly choose which mutations to apply.  
A chacha random number generator is used for this.  
It is seeded with a 64 digit hexadecimal seed.  
The seed may be provided to the app via file or CLI argument.  
If it is not provided, a seed will be generated for you.  
The app will randomly choose how many rows of the TSV file to select and then randomly choose those rows until reaching that amount, while also randomly choosing a single permutation cell from each row to be the one that is applied against the pattern cell of that row.  

Example, here it could choose anywhere between 1 and 6 rows and either the first or second permutation cell of each row( note: this is an example, the amount of permutations per row do not need to match each other and can vary ):  
```
int a = 0;		int a = 10;		int a = 20;  
int b = 1;		int b = 21;		int b = 31;
int c = 2;		int c = 32;		int c = 42;  
int d = 3;		int d = 43;		int d = 53;  
int e = 4;		int e = 54;		int e = 64;  
int f = 5;		int f = 65;		int f = 75;
```  
Below are 2 possible selections that could be made as 1<= amount selected <= 6( total available ) and the selection of rows and permuatation cells does not follow a specific formula:
```
int c = 2;		int c = 42;  
int e = 4;		int e = 54;  
int f = 5;		int f = 75; 
```
or
```
int a = 0;		int a = 20;
int b = 1;		int b = 21;
int c = 2;		int c = 32;
int d = 3;		int d = 43;
int e = 4;		int e = 64;
```
You may also specify the minimum, maximum or exact amount of random mutations you wish to be selected via arguments to the command line.  
As well you may specify a file to which the seed to be used will be output in case you wish to repeat the exact same mutations again.

#### Grouping patterns in TSV file to be selected together
If you specify a row as being part of a group, if that row is randomly selected then the entire group will be selected.
In this case as soon as the quantity of selections is greater or equal to the predetermined `mutCount` variable, then no further selections are made.  
To begin a group you place a single `^` at the beginning of a pattern cell( inside of the quote if a quoted cell )and the row above it will become the group leader to which all other rows in the group will be associated to. Here the row `int a = 0;		int a = 10;		int a = 20;` is a group leader and the two rows below it are grouped with it, whereas the row `int d = 3;   int d = 43;     int d = 53;` and anything beneath it are not part of the group:
```
bool isGood = false;    bool isGood = true;
int a = 0;		int a = 10;		int a = 20;  
^int b = 1;		int b = 21;		int b = 31;
^int c = 2;		int c = 32;		int c = 42;  
int d = 3;        int d = 43;      int d = 53;

```
The content of the group leader's pattern cell must not begin however with either of the following special characters `{'^', '@'}` as those are reserved for the beginning of cells that are grouped to a leader.  
The `@` is like `^` but locks into using the same nth permutation column as the group leader mutation. In the case of the group leader's row having more columns than the synced row and one of those columns being chosen then the last column of the synced row will be chosen. If vice versa then the additional columns of the synced row will never be selected.  
Here, if `int a = 10;` is selected for the group leader then `int c = 32;` will be selected for the bottom row's permutation.  
If `int a = 30;` or `int a = 40;` is selected for the group leader then `int c = 52;` will be selected for the bottom row's permutation.
```
int a = 0;		int a = 10;		int a = 20;      int a = 30;       int a = 40;
^int b = 1;		int b = 21;		int b = 31;
@int c = 2;		int c = 32;		int c = 42;       int c = 52;
```

### CLI Commands
```
mutate:
  -s, --seed=HEXSTRING     Pass seed in as CLI argument. Defaults to generating a new seed
  -r, --read-seed=FILE     Read PRNG seed from this file. Defaults to generating a new seed
  -w, --write-seed=FILE    Write PRNG seed out to this file. Defaults to discarding the seed
  -c, --count=NUMBER       Number of mutations to perform. Defaults to a random number of mutations
      --min-count=NUMBER   Minimum number of mutations to perform. Defaults to 1
      --max-count=NUMBER   Maximum number of mutations to perform. Defaults to the available number of mutations

  -F, --force              Overwrite existing file specified for mutated output. Defaults to aborting if output file already exists

  NOTE: The options --read-seed and --seed are mutally exclusive. You can't use both at the same time.
  NOTE: The groups --count and --min-count/--max-count are mutally exclusive. You can't specify --count if you specify --min-count or --max-count
  NOTE: If both --input and --mutations are unspecified, then the first line from stdin is swallowed and used to separate --input and --mutations

highlight:
  -f, --format             Format of the output file. One of html, srctext, or tsvtext. Defaults to html

score:
  (no special options for score)

validate:
  (no special options for validate)

Common options:
  -i, --input=FILE         Source code file to apply mutations to. Defaults to stdin
  -m, --mutations=FILE     Mutations TSV file containing mutations. Defaults to stdin
  -o, --output=FILE        Write mutated source code to this file. Defaults to stdout
  -h, --help               Show this help page
  -V, --license            Show license and version information

E.x.: mutateplaceholder mutate --input code.c --mutations muts.tsv --output output.c
```
