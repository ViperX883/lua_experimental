print("=========")
i = vault.bar()

i.a = "foo"
i.b.a = "bar"

i:print()

m = vault.map()

m:set("baz", "buz")
print("BBBB", m:get("baz"))
print("=========")
