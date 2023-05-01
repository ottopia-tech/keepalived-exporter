export SCRIPTS_DIR=$(DESTDIR)/keepalived-exporter/scripts

install: notify.sh reliability_metrics.py keepalived_configurator.py ## install dependencies for production
	install -d $(SCRIPTS_DIR)
	install -m 644 notify.sh $(SCRIPTS_DIR)
	install -m 644 reliability_metrics.py $(SCRIPTS_DIR)
	install -m 644 keepalived_configurator.py $(SCRIPTS_DIR)

clean: ## Remove previous build and release files
	@rm -rf $(SCRIPTS_DIR)
