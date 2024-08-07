// VV WPA support
const CACHE_NAME = `Resonate`;

// Use the install event to pre-cache all initial resources.
self.addEventListener('install', event => {
	event.waitUntil((async () => {
		const cache = await caches.open(CACHE_NAME);
		cache.addAll([
			'./Resonate.js',
			'./Resonate.wasm',
			'./Resonate.html',
			'./icons/ResonateIcon.png',
			'./icons/ResonateIconLarge.png',
			'./icons/ResonateIconUnsaved.png',
			'./plugins/audiostretchworker.js',
			'./plugins/timestretch.js',
			'./public/plugins/RubberBand.js',
			'./public/plugins/paulstretch.js',
			'./public/plugins/VexWarp/tools.js',
			'./public/plugins/VexWarp/stretch.js',
			'./public/plugins/VexWarp/require.js',
			'./public/plugins/VexWarp/main.js',
			'./public/plugins/VexWarp/jquery-2.0.3.js',
			'./public/plugins/VexWarp/dsp.js',
		]);
	})());
});

self.addEventListener('fetch', event => {
	event.respondWith((async () => {
		const cache = await caches.open(CACHE_NAME);

		//const injectHeaders = (request)=>{
		//	const headers = new Headers(request.headers);
		//	headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
		//	headers.set('Cross-Origin-Opener-Policy', 'same-origin');
		//	const newRequest = new Request(request, {
		//		mode: request.mode,
		//		credentials: request.credentials,
		//		headers: headers
		//	});
		//	return newRequest;
		//};

		// Get the resource from the cache.
		//const cachedResponse = await cache.match(event.request);
		//if (cachedResponse) {
		//	return cachedResponse;
		//} else {
			try {
				// If the resource was not in the cache, try the network.
				//const fetchResponse = await fetch(injectHeaders(event.request));
				const fetchResponse = await fetch(event.request);

				if(event.request.method != 'POST'){
					// Save the resource in the cache and return it.
					cache.put(event.request, fetchResponse.clone());
				}
				return fetchResponse;
			} catch (e) {
				// The network failed.
				const cachedResponse = await cache.match(event.request);
				if (cachedResponse) {
					return cachedResponse;
				}
			}
		//}
	})());
});