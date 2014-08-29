var events = require('./build/Release/NEventEmitter');

var foo = new events.eventEmitter();

//function Bar() {
//  events.eventEmitter.call(this);
//  this.x = 0; // TODO <- SEGFAULT
//}
//var foo = new Bar();

foo.on('_dispose', function() {
  console.log('Foo() is cleaned!');
});

foo.on('close', function(msg) {
  console.log('Close:', msg);
  foo.dispose();
});

var onOpen = function(msg) {
  console.log('Open:', msg);
  foo.emit('close', msg);
};

//foo.on('open', onOpen);
foo.addListener('open', onOpen);

foo.__proto__.Hello = function() {
  console.log('Hello From Function()');
};

console.log('Open Listener Count:', foo.listenerCount('open'));
console.log('Listeners:', foo.listeners('open'));
//foo.off('open', onOpen);
//foo.removeListener('open', onOpen);
//foo.removeAllListeners('open');

foo.emit('open', 'Hello Emitter!');
foo.Hello();

console.log('Foo() proto:', foo.__proto__);

var test = new events.native();

test.on('close', function(msg) {
  console.log('Message from Native:', msg);
  test.dispose();
});

test.emit('open');

console.log('Test:', test.newMsg());
console.log(test.__proto__);
