@0x9691ca7b27c42ac5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("rbk4");


struct MessageLaser {
  header @0 :MessageHeader;
  beams  @1 :List(MessageBeam);
}

struct MessageHeader {
  channel   @0 :Text;
  timestamp @1 :UInt64;
}

struct MessageBeam {
  angle @0 :Float32;
  valid @1 :Bool;
}
