# Vivaldi #

Vivaldi is a simple interpreted language inspired by Ruby, Python, Lisp, etc.

## Overview ##

### Basics ###

Vivaldi can be run either from a file or from the REPL:

<pre><code>
    $ cat test.vv
    puts("Hello, world!")
    $ vivaldi ./test.vv
    Hello, world!
    $ ./vivaldi
    >>> puts("Hello, REPL!")
    Hello, REPL!
    => nil
    >>> quit()
    $
</code></pre>

### Builtins ###

Vivaldi has a fairly limited set of builtin types:

#### Nil ####
`nil` - like `nil` in Ruby and Lisp, or `None` in Python.

#### Bools ####
`true` and `false`. What'd you expect?

#### Floats ####
64-bit floating-point values.

#### Integers ####
32-bit signed integers.

#### Strings ####
Simple, mutable string class.

#### Symbols ####
`'symbol_name` - as in Ruby or Lisp.

#### Arrays ####
Simple mutable array type:

<pre><code>
    let array = [1, 2, 3]
    let three = array.size()
    array.append('foo)
    let four = array.size()
    let two = array.at(1)
</code></pre>

#### Functions ####
Functions! Syntactically, a function is very simple:

<pre><code>
    fn <name>(<arguments>): <body>
</code></pre>

A function body is any valid Vivaldi expression (which is to say any valid
Vivaldi code):

<pre><code>
    fn is_three(x): x == 3
</code></pre>

A lambda is identical to a function, just without the name--- or, more
accurately, a function is just a lambda with a name. The following:

<pre><code>
    let five_returner = fn(x): 5
</code></pre>

is completely identical to

<pre><code>
    fn five_returner(): 5
</code></pre>

Once defined, functions work more or less like in Python:

<pre><code>
    fn id(x): x

    let function = id
    function(1) // 1
</code></pre>

#### Types ####
Everything in Vivaldi is an object, and has

* Members:

<pre><code>
    // a is an object
    a.foo = 5
    let five = a.foo
</code></pre>

* Methods (actually just members that happen to be functions):

<pre><code>
    // a is an object
    a.foo = 5
    a.bar = fn(): self.foo
    let five = a.bar()
</code></pre>

* A type

<pre><code>
    let int_type = type(int)
    int_type == Integer
</code></pre>

Defining custom types is simple:

<pre><code>
    newtype MyType {
      mem init(x): self.x = x
      mem x_is_equal_to(y): self.x = y
    }

    let my_obj = MyType(5)
    let yes = my_obj.x_is_equal_to(5)
    let no = my_obj.x_is_equal_to(47)
</code></pre>

### Structures ###

Vivaldi expressions comprise:

#### Literals ####
`5`, `5.0`, `true`, `nil`, `"string"`, `'symbol`, `['array]`, and so forth.

#### Variables, Declarations and Assignments ####

All variables must be declared before use:

<pre><code>
    i = 5     // wrong --- i hasn't been declared yet
    let i = 5 // right
    i = 3     // right --- i has been declared, so assigning to it is OK
</code></pre>

#### Control Flow ####

Vivaldi's basic control flow structures are cond statements and while loops.

<pre><code>
    let i = cond { false: "not me!", true: "me!" }
    while true: puts("looping endlessly...")
</code></pre>

Cond statements work as in Lisp: the first member of each pair is evaluated
until one is truthy (i.e. not `false` or `nil`). When the truthy member is
found, the second half of that pair is evaluated and returned. If no truthy
tests are found, the statement returns `nil`.

Simple `if` statements are provided as syntax sugar:

<pre><code>
    // i is a variable
    let a = if i: "i is truthy"
    if a == nil: puts("i isn't truthy")

    // equivalent:
    let a = cond { i: "i is truthy" }
    cond { a == nil: puts("i isn't truthy") }
</code></pre>

while loops are very straightforward:

<pre><code>
    while <condition>: <expression>
</code></pre>

Note that while loops always evaluate to `nil` or `false`--- whatever the
condition evaluated to when the loop broke.

#### Blocks ####

Functions, cond/if statements, and while loops are all limited to a single
expression for their bodies. This kind of sucks if you want to do anything
actually interesting with them. Fortunately, it's possible, using blocks, to
mash a bunch of expressions together in a sequence:

<pre><code>
    let i = {
      puts("I'm in a block!")
      5; 4; 3; 2; 1
    }
    i == 1
</code></pre>

Using this, it's possible to build actually useful constructs:

<pre><code>
    fn for_each(array, func): {
      let i = 0
      let size = func.size()
      while i < size: {
        func(array.at(i))
        i = i + 1
      }
    }
</code></pre>

Blocks have nested scope:

<pre><code>
    { let j = 5 }
    puts(j) // wrong --- j is out of scope

    let j = 5
    { puts(j) } // fine
</code></pre>

#### Exceptions ####

Like in C++, exceptions in Vivaldi don't have any special type (in fact there is
no builtin exception type; strings are used instead). Otherwise they work pretty
much as you'd expect:

<pre><code>
    let i = try: except 5
    catch e: e + 1
    i == 6
</code></pre>

As everywhere else in Vivaldi, the pieces of code following `try` and `catch`
are expressions.

### Example ###

#### FizzBuzz ####

<pre><code>
    let i = 0
    while (i = i + 1) < 100: cond {
      i % 15 == 0: puts("FizzBuzz"),
      i % 5 == 0:  puts("Buzz"),
      i % 3 == 0:  puts("Fizz"),
      true:        puts(i)
    }
</code></pre>

#### Basic I/O ####

<pre><code>
    print("What's your name? ")
    let name = gets()
    print("Hello, ".append(name))
</code></pre>

#### Prime Number Sieve ####

<pre><code>
    // This *will* be builtin to later versions
    fn for_each(array, func): {
      let i = 0
      let size = func.size()
      while i < size: {
        func(array.at(i))
        i = i + 1
      }
    }

    fn filter(array, pred): {
      let new_arr = []
      for_each(array, fn (i): if pred(i): new_arr.append(i))
      new_arr
    }

    fn range(start, finish): {
      let arr = [start]
      while start < finish: arr.append(start = start + 1)
      arr
    }

    fn primes(upto): {
      let found_primes = []
      let candidates = range(2, upto)
      while candidates.size() > 0: {
        let prime = candidates.at(0)
        found_primes.append(prime)
        candidates = filter(candidates, fn(x): x % prime != 0)
      }
      found_primes
    }

    puts(primes(1000))
</code></pre>
