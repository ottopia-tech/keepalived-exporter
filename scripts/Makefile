export SCRIPTS_DIR=$(DESTDIR)/opt/ottopia/keepalived-exporter/scripts

install: notify.sh reliability_metrics.py ## install dependencies for production
	install -d $(SCRIPTS_DIR)
	install -m 755 notify.sh $(SCRIPTS_DIR)
	install -m 755 reliability_metrics.py $(SCRIPTS_DIR)

clean: ## Remove previous build and release files
	@rm -rf $(SCRIPTS_DIR)

