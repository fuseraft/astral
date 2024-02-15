# Console I/O

Kiwi offers the following constructs to work with console I/O:
- `input(msg)`: Read input from the console.
- `print`: Print text to the console.
- `println`: Print a line of text to the console.

For file I/O, please see [@kiwi/fileio](kiwilib/fileio.md).

### `input()`

To read input from the console, use the `input()` builtin.

```ruby
print "Enter your favorite fruit: "
@fruit = input()
# @fruit contains "kiwi"

println "Your favorite fruit is ${@fruit}"
# prints: Your favorite fruit is kiwi
```


### `print`

To print text to the console, use the `print` keyword.

```ruby
print "Hello"
print ", World!" # prints: Hello, World!"
```

### `println`

To print a line of text to the console, use the `println` keyword.

```ruby
println "Do you like programming?"
println "Let's build something amazing!"

# prints: 
# Do you like programming?
# Let's build something amazing!
```