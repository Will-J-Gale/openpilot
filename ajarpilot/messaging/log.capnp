@0xa7e52474cc36f75b;

#Create single Log struct with union of all types
#Log has timestamp

struct XYZData @0x9574fe79216448bb {
  x @0 :Float32;
  y @1 :Float32;
  z @2 :Float32;
  xStd @3 :Float32;
  yStd @4 :Float32;
  zStd @5 :Float32;
}

struct ModelData @0xf501e46716f36a17 {
  leadCar @0 :List(Float32);
  path @1 :List(XYZData);
}