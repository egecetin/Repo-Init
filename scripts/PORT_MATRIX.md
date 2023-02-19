Configured firewalld rules with system-setup.sh,

| Application | Port | Zone |
|:-----------:|:----:|:----:|
| SSH | 22 | Public |
| Grafana | 3000 | Trusted |
| Loki | 3100 | Trusted |
| Prometheus | 9000 | Trusted |
| Promtail | 9080 | Trusted |
| Cockpit | 9090 | Public |
| Node exporter | 9100 | Trusted |
