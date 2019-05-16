# tokenmark.io client

Tokenmark.io is a small desktop based software to watch cryptocurrencies markets in real-time.
It is written entirely with C++ and Qt framework.

You can find more about the software with a video capture of the running software:
https://www.youtube.com/watch?v=LFsa4JeFRaQ

Supported exchanges: Binance, Bitfinex, Bitstamp, Coinbase Pro, Huobi, Kraken and OKEx.

At first I wrote this app for private purposes and never expected to release it someday, however a good friend convinced me to release it, this explain the lack of comments in the code.

## building

Building the client is very easy, just open Qt creator, configure your toolkit and build.
Tested version of toolkits are 5.9.2 and 5.12.3, but it should work with any version above 5.9 without changes.
The application can also be built using command line (refer to Qt help for more inormation that).
I work mainly on Linux OS for dev, so the application have not been tested in a Windows environment (but tested and works on my virtual box VM).

## known issues

Not handling well retina displays.
