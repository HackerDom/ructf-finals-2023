# RuCTF Finals 2023 | Sneakers

## Description
At Sneaker Art Gallery, you can create truly unique and personalized "barkhatnye tyagi" and also browse through an extensive collection of sneaker images from various styles.

The flag is located in the sneaker image, and the only way to obtain it is by downloading the image using `SneakersToken`.

# Original vuln
You can surf through the gallery and fetch the `SneakersToken`. The filtering system makes only sneakers with the `Pixel` collection visible to everyone. However, there is a bug in the `FilterReducer` that can delete redundant filters. In all filters, the default comparison uses the reference. The `AdminFilter` is a `record` with overridden `Equals()` and `GetHashCode()` methods, which means that `FilterReducer` can delete all occurrences of `AdminFilter`. You need to force `FilterReducer` to do it.

Type of `AdminFilter` is not represented in the `FilterType` enumeration. However, ASP.NET core doesn't have strong enum validation in the request body. For example:
```
Enum.TryParse("BLABLABLA", typeof(FilterType), out _); // false

Enum.TryParse("-100500", typeof(FilterType), out _); // true (FilterType it's integer inder the hood)
```

# Attack 
`FLAG_ID` - known `SneakersId` from cs

* Create artist to pass auth
* Send `gallery/exhibits` with `FLAG_ID` and filters payload.  
* Download image using `SneakersToken` and get flag

Filters payload example:
```
"conditions": [
    {
		"type" : "-100500",
      	"jsonString" : "{ \"OwnerToken\" : { \"Value\" : \"<your artist token>\"} }"
    },
    {
		"type" : "-100500",
      	"jsonString" : "{ \"OwnerToken\" : { \"Value\" : \"<your artist token>\"} }"
    }
  ]
```

# Defense
Make `AdminFilter` `class`, not `record`. 
