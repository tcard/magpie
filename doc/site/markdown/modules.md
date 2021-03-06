^title Modules

Once you start writing programs that are more than little toys, you quickly run into two problems:

1. You want to break them down into multiple smaller files to make it easier to
   find your way around them.
2. You want to reuse pieces of them across different programs.

To address those, Magpie has a simple *module* system. A file containing Magpie code saved with a `.mag` extension defines a module. A module can use the code defined in another module by *importing* it. You can break big programs into smaller modules that import each other, and you can reuse code by having multiple programs share the use of a single module.

## Importing a Module

One module can import another using an `import` expression. Like any expression, these can appear anywhere, but the convention is to place them at the top of the file before any other code.

    :::magpie
    import io.path

Here, we're importing the `io.path` module. Importing a module does two things: it runs the module's code, then it binds name in the importing scope.

## Qualified Imports

By default, when you import a module you get everything it exports just as it appears in that module. That works most of the time, but sometimes you may want finer-grained control. Maybe you only want to import a few things, or you need to rename something to deal with a name collision.

To handle those, Magpie has a few additional qualifiers you can provide when you import. The first is a prefix:

    :::magpie
    import io.path as path

When you add `as` followed by a name after the module being imported, it prefixes every imported name with that name followed by a dot (`.`). So if `io.path` defines a `baseName` method, we will import it as `path.baseName`. This is useful if you want to bring in every name, but make it very clear where they are coming from.

If you want more precise control over specific names, you can add a `with` block to the import:

    :::magpie
    import io.path with
        ...
    end

Each line inside that block lets you do a few things with a name. First is renaming:

    :::magpie
    import io.path with
        baseName as base
    end

Using `as` lets you provide a new name for a name that you're importing. With the above example, this module would be able able to invoke the `baseName` method defined in `io.path` by calling `base`.

If a line starts with `export` it lets you re-export an imported name.

    :::magpie
    import io.path with
        export baseName
    end

By default, imported names are not in turn exported for a module. The idea is that you import things to use yourself, but you only export the behavior you define in your module.

If you do actually want to make something you import part of the exported set of names in your module you can add `export` as shown. Here, anyone importing your module will be able to call the `baseName` method that's defined in `io.path` as if it were defined in your module.

<p class="future">It isn't implemented yet, but at some point you'll also be able to exclude imported names and indicate that you only want to import a select set of names.</p>

## Module Loading

When a module is imported for the first time, Magpie needs to find the file that corresponds to that module's name. This is called the *lookup* process. It works like so:

1. Dots in the name are turned into directory separators. So if we're loading `io.path`, we'll turn that into `io/path`.
2. A `.mag` file extension is added, so now we have `io/path.mag`.
3. We look for a file at that path relative to the current working directory. If found there, we're done.
4. Otherwise, we look for it inside the standard Magpie `lib/` directory which lives where the main executable is.

If Magpie couldn't find a module at either of those paths, an [error](error-handling.html) is thrown.

If it succeeds, Magpie creates a new empty module scope and runs the loaded code. Magpie has no single global scope where names are defined. Instead, each module has its own top level scope. When you define a variable or method in a module, that name is only visible to the module where it's defined.

When that's done, the module is successfully loaded. Note that Magpie only does this the *first* time a given module is imported. If a module is imported by three other modules, its code will only be run once, and it will only have a single top-level scope in memory.

Once a module has been loaded in response to an `import`, it proceeds to the next step, importing names.

## Importing Names

When one module imports another, it usually does so because it wants to use some of the methods or variables defined in that module. But, since there is no single global scope, it doesn't have a way to get at them.

To solve that, an `import` expression will also define names in the importing module that reference [methods](multimethods.html) and [variables](variables.html) defined in the imported module. By default, when you import a module, any methods and variables that that module declares at its top level (i.e. not inside some nested [block](blocks.html) scope or in a method) will be imported into yours. For example:

    :::magpie
    // dessert.mag
    var pie = "apple"
    def eatPie()
        print("You eat a delicious " + pie + " pie")
    end

    // hungry.mag
    import dessert

    print("I imported " + pie)
    eatPie()

When `hungry.mag` imports `dessert.mag` it gets a variable named `pie` defined in *its* scope that references the same value that it has in `dessert.mag`. Likewise, it gets a top-level `eatPie()` defined in its scope that it can then call.

It's important to realize that when you import a variable, you get your *own* variable declared in your module that points to the same value that the exported variable had *when you imported it*. If either module assigns a different value to it, the other won't see that change. Consider:

    :::magpie
    // dessert.mag
    var pie = "apple"
    def eatPie()
        print("You eat a delicious " + pie + " pie")
    end

    def changePie()
        pie = "chocolate"
    end

    // hungry.mag
    import dessert

    changePie()
    print(pie) // Still prints "apple"
    eatPie() // Prints "chocolate"

Here, `dessert.mag` is changing the value of `pie` after `hungry.mag` imports it. We won't see that change reflected in the `pie` variable defined in `hungry.mag`, but we will see it when it calls `eatPie()` since that method looks it up in `dessert.mag` where its defined.

## Public and Private Names

By default, any variable or method defined at the top level of a module is considered "public" and can be exported into other modules. Sometimes, though, you want to define code that the module can use itself but that isn't visible to the outside world.

To address that, Magpie supports *private names*. Any name that starts with an underscore will not be exported from a module.

    :::magpie
    // secret.mag
    val _hidden = "you can't see me!"

    // main.mag
    import secret

    print(_hidden) // ERROR! _hidden is not defined

This applies to methods, variables, as well as [classes](classes.html) and their fields. With a class, you can even make some fields private and others public.

    :::magpie
    // secret.mag
    var _nextKey = 12345

    defclass Lock
        val _key = _nextKey = _nextKey + 1
    end

    // main.mag
    import secret

    val lock = Lock new()
    lock _key // ERROR: _key getter isn't defined here

**TODO: circular dependencies, _init.mag, relative imports.**
