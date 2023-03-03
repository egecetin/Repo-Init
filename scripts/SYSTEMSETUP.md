system-setup.sh prepares and install applications to RedHat based systems.

It installs the following packages,

- [Cockpit](https://cockpit-project.org/) and [Cockpit Navigator](https://github.com/45Drives/cockpit-navigator)
- [Grafana](https://grafana.com/)
- [Loki](https://grafana.com/oss/loki/) and [Promtail](https://grafana.com/docs/loki/latest/clients/promtail/)
- [Prometheus](https://prometheus.io/) and [Node exporter](https://github.com/prometheus/node_exporter)
- and more (please check script for full list)

Adds following firewalld rules,

| Application | Port | Zone |
|:-----------:|:----:|:----:|
| SSH | 22 | Public |
| Grafana | 3000 | Trusted |
| Loki | 3100 | Trusted |
| Prometheus | 9000 | Trusted |
| Promtail | 9080 | Trusted |
| Cockpit | 9090 | Public |
| Node exporter | 9100 | Trusted |

Optionally you can import scripts/data/GrafanaDasboard.json to Grafana to display basic metrics about your system
