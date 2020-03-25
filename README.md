## Geosick

Geosick is a library written for the Covid19cz and ZostanZdravy initiatives by Jan Plhák
(jan.plhak@kiwi.com) and Jan Špaček (patek.mail@gmail.com) from
[distributa.io](https://www.distributa.io). The purpose of the library is to provide
functionality for GPS sequencies analysis to determine the probability of a healthy person
getting infected by someone sick (e.g. infected by the Covid19 virus). We provide two main
services.

# HTTP Server

Our HTTP server provides simple API for evaluating two sequencies of GPS locations and returns
a probability with which one person was infected by another. More info in the
[HTTP server docs](docs/http_server.md).

# Batch processing

For processing of a large number of users and their GPS sequences, we provide a C++ tool that is
capable of evaluating hundreds of thousands of user GPS sequences, comparing them with GPS
sequences of sick people and reporting all possibly infected users. For more details, please
refer to our [batch processing docs](docs/batch_processing.md). If you would like to use it,
please reach out to us using the contacts listed above and we will assist you with the integration
of our tool into your setup.
