# lansound

A PulseAudio sink and server program for streaming low-latency audio across a LAN.

Not properly packaged yet. To use, clone this repo on both the server and client. On the
client, run `make` in the paswitch directory. Copy the resulting `paswitch` executable
to the parent directory.

On both the client and server you will need pyzmq, gstreamer, and the
gobject-introspection Python bindings for gstreamer and glib.

Run `./lasound-server` on the server, and `./lansound` on the client. Supported
arguments on the client are:

Possible client command line options are:
```
$ ./lansound -h
usage: lansound [-h] [--list] [-H HOST] [-p PORT] [-l LATENCY]
                [-i IDLE_TIMEOUT] [-n] [-4] [-v]

Streams all your computer's audio output to a server running lansound-server.

optional arguments:
  -h, --help            show this help message and exit
  --list                List all lansound servers on the network and exit.
  -H HOST, --host HOST  The hostname or IP address of the lansound-server to
                        connect to. If not given, the first server found on
                        the local network will be used.
  -p PORT, --port PORT  If --server is given, the port to connect to. Defaults
                        to 57066.
  -l LATENCY, --latency LATENCY
                        The duration, in milliseconds, to buffer the input
                        data before playing it on the server. This increases
                        latency, but also makes the audio less likely to cut
                        out if there is a glitch in the network. The default
                        is 400ms, which is appropriate for streaming over a
                        local network.
  -i IDLE_TIMEOUT, --idle-timeout IDLE_TIMEOUT
                        How long in milliseconds to wait after audio stops
                        playing before pausing transmission of data to the
                        server. This is useful for conserving battery, CPU and
                        bandwidth when there is no audio. Defaults to 5000ms.
  -n, --no-auto-enable  Do not automatically switch to the PulseAudio sink
                        that lansound creates. Audio output will continue from
                        whatever device is currently selected until the user
                        selects the lansound sink as their default PulseAudio
                        sink in gnome sound settings or equivalent. Or, if
                        using a more advanced tool such as pavucontrol, the
                        user can direct, for example, only one application's
                        output to lansound.
  -4, --ipv4            Only use IPv4 when choosing a server or looking up
                        given hostname. Otherwise, IPv6 is preferred.
  -v, --verbose         Print debug information.
```


Possible server command line options are:

```
$ ./lansound-server -h
usage: lansound-server [-h] [-n NAME] [-p PORT] [-m] [-l LATENCY]

Server for lansound. Listens on a ZMQ socket for uncompressed CD quality
audio, and outputs it to the default pulseaudio sink. Buffers playback with a
configurable buffer, to ensure smooth playback in the face of small
interruptions to the stream, at the cost of some latency.

optional arguments:
  -h, --help            show this help message and exit
  -n NAME, --name NAME  The name to give the server. This is how it will
                        appear to clients on the network. Default: 'lansound
                        server'
  -p PORT, --port PORT  The port to listen on. Default: 57066
  -m, --monitor         Print the buffer level and number of buffer underruns
                        to the terminal, in order to visualise or monitor how
                        stable the stream is.
  -l LATENCY, --default-latency LATENCY
                        The default duration, in milliseconds, to buffer the
                        input data before playing it. This increases latency,
                        but also makes the audio less likely to cut out if
                        there is a glitch in the input stream. The default is
                        400ms, which is appropriate for streaming over a local
                        network. Clients can change this, the value set here
                        is only the default, used until one is set by a client
```

# TODO:

* Write the functionality provided by `paswitch` in Python
* Properly document dependencies and how to install them on different distros
* Wrap into a standard Python package that can be installed with `pip`
* Maybe write an appindicator showing the status and allowing you to run without a
  terminal window always present