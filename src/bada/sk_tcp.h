static void sk_tcp_set_frozen(::Socket sock, int is_frozen) {
	Actual_Socket s = (Actual_Socket) sock;
	if (s->frozen == is_frozen)
		return;
	s->frozen = is_frozen;
	if (!is_frozen)
		skynet->DoSelect(1);
	s->frozen_readable = 0;
}
