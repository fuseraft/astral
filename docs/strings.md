# Strings

The `String` type in Kiwi is used to represent text data.

Strings in Kiwi are wrapped with either double-quotes or single-quotes.

### Defining a String

A string wrapped in double-quotes can contain string interpolation expressions.

```ruby
hello = "Hello, World!"
println(hello) # prints: Hello, World!
```

### Defining Raw Strings

A string wrapped in single-quotes is treated as a raw string. This is useful to avoid double-escaping characters used in regular expressions.

```ruby
some_text = "my email: example@test.com" # a regular string
regex = '\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z0-9]{2,}\b' # a raw string
println(some_text.find(regex))  # prints: example@test.com
```

### String Interpolation

You can interpolate values into a string using the `${}` syntax.

```ruby
x = 50
y = 10
res = "${x} / ${y} = ${x / y}" # contains 3 interpolations: `x`, `y`, `x / y`
println(res) # prints: 50 / 10 = 5
```

### String Multiplication

You can multiply strings using the multiplication operator `*` followed by an integer value. Multiplying by `0` will return an empty string.

```ruby
strmul = "hello" * 2 
println(strmul) # prints: hellohello
```

### Builtins

For documentation on `String` builtins, take a look at the [`String` builtins](builtins.md#string-builtins).