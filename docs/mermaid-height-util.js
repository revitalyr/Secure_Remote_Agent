class MermaidHeightAdjuster {
    constructor() {
        this.currentScale = 100;
        this.scaleStep = 10;
        this.minScale = 50;
        this.maxScale = 200;
    }

    setupTabSwitching() {
        const tabs = document.querySelectorAll('.nav-tab');
        tabs.forEach(tab => {
            tab.addEventListener('click', (e) => {
                e.preventDefault();
                const targetId = tab.getAttribute('data-target');
                this.switchTab(targetId);
            });
        });
    }

    setupScaleControls() {
        document.addEventListener('click', (e) => {
            const scaleDelta = e.target.closest('[data-scale-delta]');
            const resetBtn = e.target.closest('[data-reset-scale]');

            if (scaleDelta) {
                const delta = parseInt(scaleDelta.getAttribute('data-scale-delta'));
                this.changeScale(delta);
            } else if (resetBtn) {
                this.resetScale();
            }
        });
    }

    switchTab(tabId) {
        // Hide all tab contents
        document.querySelectorAll('.tab-content').forEach(content => {
            content.classList.remove('active');
        });

        // Remove active class from all tabs
        document.querySelectorAll('.nav-tab').forEach(tab => {
            tab.classList.remove('active');
        });

        // Show selected tab content
        const targetContent = document.getElementById(tabId);
        if (targetContent) {
            targetContent.classList.add('active');

            // Render Mermaid diagram after tab is visible
            setTimeout(() => {
                this.renderDiagramInTab(tabId);
            }, 100);
        }

        // Set active tab
        const activeTab = document.querySelector(`.nav-tab[data-target="${tabId}"]`);
        if (activeTab) {
            activeTab.classList.add('active');
        }
    }

    renderDiagramInTab(tabId) {
        const container = document.getElementById(tabId);
        if (!container) return;

        const diagramDiv = container.querySelector('.mermaid');
        if (!diagramDiv) return;

        const diagramId = `${tabId}-diagram`;
        diagramDiv.id = diagramId;

        mermaid.run({
            nodes: [diagramDiv]
        }).then(() => {
            this.adjustDiagram(container);
        }).catch(err => {
            console.error('Mermaid render error:', err);
        });
    }

    adjustDiagram(container) {
        const diagramDiv = container.querySelector('.mermaid');
        if (!diagramDiv) return;

        const svg = diagramDiv.querySelector('svg');
        if (!svg) return;

        const bbox = svg.getBBox();
        const padding = 20;

        diagramDiv.style.width = `${bbox.width + padding}px`;
        diagramDiv.style.height = `${bbox.height + padding}px`;
        svg.style.width = '100%';
        svg.style.height = '100%';

        // Apply current scale
        this.applyScaleToDiagram(diagramDiv);
    }

    changeScale(delta) {
        const newScale = this.currentScale + delta;
        if (newScale >= this.minScale && newScale <= this.maxScale) {
            this.currentScale = newScale;
            this.applyScaleToAllDiagrams();
            this.updateScaleDisplay();
        }
    }

    resetScale() {
        this.currentScale = 100;
        this.applyScaleToAllDiagrams();
        this.updateScaleDisplay();
    }

    applyScaleToAllDiagrams() {
        document.querySelectorAll('.tab-content.active .mermaid').forEach(diagramDiv => {
            this.applyScaleToDiagram(diagramDiv);
        });
    }

    applyScaleToDiagram(diagramDiv) {
        const svg = diagramDiv.querySelector('svg');
        if (!svg) return;

        svg.style.transform = `scale(${this.currentScale / 100})`;
        svg.style.transformOrigin = 'top left';
        diagramDiv.style.overflow = 'auto';
    }

    updateScaleDisplay() {
        const display = document.querySelector('.scale-display');
        if (display) {
            display.textContent = `${this.currentScale}%`;
        }
    }

    async initialize() {
        await mermaid.initialize({
            startOnLoad: false,
            theme: 'default',
            themeVariables: {
                primaryColor: '#667eea',
                primaryTextColor: '#333',
                lineColor: '#667eea',
                secondaryColor: '#764ba2',
                fontSize: '14px'
            },
            flowchart: { useMaxWidth: false, htmlLabels: true }
        });

        this.setupTabSwitching();
        this.setupScaleControls();

        // Render first tab by default
        const firstTab = document.querySelector('.nav-tab');
        if (firstTab) {
            const targetId = firstTab.getAttribute('data-target');
            setTimeout(() => {
                this.renderDiagramInTab(targetId);
            }, 100);
        }
    }
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    const adjuster = new MermaidHeightAdjuster();
    adjuster.initialize();
});
